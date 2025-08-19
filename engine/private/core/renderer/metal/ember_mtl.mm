#import "core/renderer/metal/ember_mtl.h"

#import <SDL3/SDL.h>
#import <SDL3/SDL_metal.h>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <stb_image.h>



MetalRenderer::MetalRenderer() = default;
MetalRenderer::~MetalRenderer() = default;

void MetalRenderer::initialize() {
    Type = Backend::METAL;

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) { LOG_INFO("Failed to create Metal device, reason unknown."); return; }
    device_ = (__bridge void*)device;

    id<MTLCommandQueue> queue = [device newCommandQueue];
    queue_ = (__bridge_retained void*)queue;

    metal_view_ = SDL_Metal_CreateView(Window);
    if (!metal_view_) { LOG_INFO("Failed to create Metal view: %s", SDL_GetError()); return; }

    CAMetalLayer* layer = (__bridge CAMetalLayer*)SDL_Metal_GetLayer(metal_view_);
    layer.device = device;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    layer.framebufferOnly = YES;
    layer.drawableSize = CGSizeMake(Viewport[0], Viewport[1]);
    metal_layer_ = (__bridge void*)layer;

    NSError* error = nil;
    id<MTLLibrary> library = [device newDefaultLibrary];
    if (!library) { SDL_Log("Failed to load default Metal Shader (not compiled the default.metal)"); return; }

    id<MTLFunction> vertFunc = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragFunc = [library newFunctionWithName:@"fragment_main"];
    if (!vertFunc || !fragFunc) { SDL_Log("Missing Metal shader functions "); return; }

    MTLVertexDescriptor* vdesc = [[MTLVertexDescriptor alloc] init];
    vdesc.attributes[0].format = MTLVertexFormatFloat2;
    vdesc.attributes[0].offset = offsetof(Vertex, position);
    vdesc.attributes[0].bufferIndex = 0;

    vdesc.attributes[1].format = MTLVertexFormatFloat2;
    vdesc.attributes[1].offset = offsetof(Vertex, tex_coord);
    vdesc.attributes[1].bufferIndex = 0;

    vdesc.attributes[2].format = MTLVertexFormatFloat4;
    vdesc.attributes[2].offset = offsetof(Vertex, color);
    vdesc.attributes[2].bufferIndex = 0;

    vdesc.layouts[0].stride = sizeof(Vertex);
    vdesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

    MTLRenderPipelineDescriptor* pdesc = [[MTLRenderPipelineDescriptor alloc] init];
    pdesc.vertexFunction = vertFunc;
    pdesc.fragmentFunction = fragFunc;
    pdesc.vertexDescriptor = vdesc;
    pdesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    pdesc.colorAttachments[0].blendingEnabled = YES;
    pdesc.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
    pdesc.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
    pdesc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
    pdesc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    pdesc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
    pdesc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

    id<MTLRenderPipelineState> pso = [device newRenderPipelineStateWithDescriptor:pdesc error:&error];
    if (error || !pso) {
        SDL_Log("Metal PipelineState creation failed: %s", [[error localizedDescription] UTF8String]);
        return;
    }
    pipeline_ = (__bridge_retained void*)pso;

    MTLSamplerDescriptor* sdesc = [[MTLSamplerDescriptor alloc] init];
    sdesc.minFilter = MTLSamplerMinMagFilterNearest;
    sdesc.magFilter = MTLSamplerMinMagFilterNearest;
    sdesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
    sdesc.tAddressMode = MTLSamplerAddressModeClampToEdge;
    id<MTLSamplerState> sstate = [device newSamplerStateWithDescriptor:sdesc];
    sampler_ = (__bridge_retained void*)sstate;

    projection = glm::ortho(0.0f, float(Viewport[0]), float(Viewport[1]), 0.0f, -1.0f, 1.0f);

}

void MetalRenderer::clear(glm::vec4 color) {
    commands.clear();
    batches.clear();

    if (color == glm::vec4(0.0f)) {
        color = GEngine->Config.get_environment().clear_color;
    }

    clear_color_ = color;
}

std::shared_ptr<Texture> MetalRenderer::load_texture(const std::string& path) {
    auto it = textures.find(path);
    if (it != textures.end()) return it->second;

    int w=0,h=0,nr_channels=4;
    stbi_set_flip_vertically_on_load(false);

    const auto buffer = _load_file_into_memory(path);

    if (buffer.empty()) {
        LOG_INFO("Failed to load texture from file: %s", path.c_str());
    }

    unsigned char* data = stbi_load_from_memory((unsigned char*)buffer.data(), (int)buffer.size(), &w, &h, &nr_channels, 4);

    bool has_error_texture = false;
    if (!data) {
        w = h = 128;
        has_error_texture = true;
        data = (unsigned char*)malloc(w*h*4);
        for (int y=0;y<h;++y) for (int x=0;x<w;++x) {
            int i=(y*w+x)*4;
            bool pink = ((x/8)%2)==((y/8)%2);
            data[i+0] = pink?180:0;
            data[i+1]= 0;
            data[i+2]= pink ? 180:0;
            data[i+3]= 255;
        }
    }

    id<MTLDevice> device = (__bridge id<MTLDevice>)device_;
    MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:w height:h mipmapped:NO];
    desc.usage = MTLTextureUsageShaderRead;
    id<MTLTexture> mtlTex = [device newTextureWithDescriptor:desc];

    if (!mtlTex) {
        if (!has_error_texture) stbi_image_free(data); else free(data);
        LOG_ERROR("Texture creation failed for %s", path.c_str());
        return {};
    }

    MTLRegion region = {{0,0,0},{(NSUInteger)w,(NSUInteger)h,1}};
    [mtlTex replaceRegion:region mipmapLevel:0 withBytes:data bytesPerRow:w*4];

    if (!has_error_texture){

        LOG_INFO("Loaded texture with ID: %d, path: %s", (Uint32)mtlTex.hash, path.c_str());
        LOG_INFO(" > Width %d, Height %d", w,h);
        LOG_INFO(" > Num. Channels %d", nr_channels);
        stbi_image_free(data);

    } else {
        LOG_WARN("Couldn't load texture from file: %s", path.c_str());

        free(data);

    }

    auto t = std::make_shared<Texture>();

    t->id = (Uint32)mtlTex.hash;
    t->width = w; t->height = h; t->path = path;
    t->mtlTexture = (__bridge_retained void*)mtlTex;

    _texture_sizes[t->id] = glm::vec2(w,h);
    textures[path] = t;
    return t;
}

void MetalRenderer::draw_rect(Rect2 rect, float rotation, const glm::vec4& color, bool filled, int z_index) {
    BatchKey key{};
    key.texture_id = 0;
    key.z_index = z_index;
    key.type = DrawCommandType::RECT;
    key.uber_shader = UberShader::none();

    Batch& batch = batches[key];
    batch.mode = DrawCommandMode::TRIANGLES;
    batch.z_index = z_index;
    batch.type = DrawCommandType::RECT;
    batch.texture_id = 0;

    glm::vec2 center(rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f);

    glm::vec2 p0 = _rotate_point({rect.x, rect.y}, center, rotation);
    glm::vec2 p1 = _rotate_point({rect.x + rect.width, rect.y}, center, rotation);
    glm::vec2 p2 = _rotate_point({rect.x, rect.y + rect.height}, center, rotation);
    glm::vec2 p3 = _rotate_point({rect.x + rect.width, rect.y + rect.height}, center, rotation);

    uint32_t base = static_cast<uint32_t>(batch.vertices.size());

    batch.vertices.push_back({p0, {0,0}, color});
    batch.vertices.push_back({p1, {0,0}, color});
    batch.vertices.push_back({p2, {0,0}, color});
    batch.vertices.push_back({p3, {0,0}, color});

    batch.indices.push_back(base + 0);
    batch.indices.push_back(base + 1);
    batch.indices.push_back(base + 2);
    batch.indices.push_back(base + 2);
    batch.indices.push_back(base + 1);
    batch.indices.push_back(base + 3);
}

void MetalRenderer::draw_texture(const Texture* texture, const Rect2& dest, float rotation,
                                 const glm::vec4& color, const Rect2& srcRect, int z_index, const UberShader& uber) {
    if (!texture || !texture->mtlTexture) return;

    float tx0 = 0.f, tx1 = 1.f, ty0 = 0.f, ty1 = 1.f;
    if (!(srcRect.width == 0 && srcRect.height == 0)) {
        tx0 = srcRect.x / float(texture->width);
        ty0 = srcRect.y / float(texture->height);
        tx1 = (srcRect.x + srcRect.width) / float(texture->width);
        ty1 = (srcRect.y + srcRect.height) / float(texture->height);
    }

    BatchKey key{};
    key.texture_id = texture->id;
    key.z_index = z_index;
    key.type = DrawCommandType::TEXTURE;
    key.uber_shader = uber;

    Batch& batch = batches[key];
    batch.mode = DrawCommandMode::TRIANGLES;
    batch.z_index = z_index;
    batch.type = DrawCommandType::TEXTURE;
    batch.texture_id = texture->id;
    batch.uber_shader = uber;

    glm::vec2 center(dest.x + dest.width * 0.5f, dest.y + dest.height * 0.5f);

    glm::vec2 p0 = _rotate_point({dest.x, dest.y}, center, rotation);
    glm::vec2 p1 = _rotate_point({dest.x + dest.width, dest.y}, center, rotation);
    glm::vec2 p2 = _rotate_point({dest.x, dest.y + dest.height}, center, rotation);
    glm::vec2 p3 = _rotate_point({dest.x + dest.width, dest.y + dest.height}, center, rotation);

    uint32_t base = static_cast<uint32_t>(batch.vertices.size());
    batch.vertices.push_back({p0, {tx0, ty0}, color});
    batch.vertices.push_back({p1, {tx1, ty0}, color});
    batch.vertices.push_back({p2, {tx0, ty1}, color});
    batch.vertices.push_back({p3, {tx1, ty1}, color});

    batch.indices.push_back(base + 0);
    batch.indices.push_back(base + 1);
    batch.indices.push_back(base + 2);
    batch.indices.push_back(base + 2);
    batch.indices.push_back(base + 1);
    batch.indices.push_back(base + 3);
}

void MetalRenderer::flush() {
    @autoreleasepool {

    if (!device_ || !queue_ || !pipeline_ || !metal_layer_) return;

    id<MTLDevice> device = (__bridge id<MTLDevice>)device_;
    id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)queue_;
    id<MTLRenderPipelineState> pipeline = (__bridge id<MTLRenderPipelineState>)pipeline_;
    id<MTLSamplerState> sampler = (__bridge id<MTLSamplerState>)sampler_;
    CAMetalLayer* layer = (__bridge CAMetalLayer*)metal_layer_;

    id<CAMetalDrawable> drawable = [layer nextDrawable];
    if (!drawable) return;

    MTLRenderPassDescriptor* passDesc = [MTLRenderPassDescriptor renderPassDescriptor];
    passDesc.colorAttachments[0].texture = drawable.texture;
    passDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
    passDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
    passDesc.colorAttachments[0].clearColor = MTLClearColorMake(clear_color_.r, clear_color_.g, clear_color_.b, clear_color_.a);

    id<MTLCommandBuffer> cmdBuffer = [queue commandBuffer];
    id<MTLRenderCommandEncoder> encoder = [cmdBuffer renderCommandEncoderWithDescriptor:passDesc];
    [encoder setRenderPipelineState:pipeline];


    for (const auto &cmd : commands) {
        BatchKey k{cmd.texture_id, cmd.z_index, cmd.type, UberShader::none()};
        Batch &b = batches[k];
        uint32_t base = static_cast<uint32_t>(b.vertices.size());
        b.vertices.insert(b.vertices.end(), cmd.vertices.begin(), cmd.vertices.end());
        for (auto idx : cmd.indices) b.indices.push_back(base + idx);
        b.texture_id = cmd.texture_id;
        b.type = cmd.type;
        b.z_index = cmd.z_index;
        b.uber_shader =UberShader::none();
    }

    std::vector<std::pair<BatchKey, Batch*>> sorted;
    sorted.reserve(batches.size());
    for (auto &kv : batches) sorted.emplace_back(kv.first, &kv.second);
    std::sort(sorted.begin(), sorted.end(), [](auto &a, auto &b){ return a.second->z_index < b.second->z_index; });

    [encoder setVertexBytes:&projection length:sizeof(projection) atIndex:1];

    struct FragUniforms { uint32_t use_texture; uint32_t pad[3]; };

    for (auto &kv : sorted) {
        Batch &batch = *kv.second;
        if (batch.vertices.empty() || batch.indices.empty()) continue;

        id<MTLBuffer> vbuf = [device newBufferWithBytes:batch.vertices.data()
                                                 length:batch.vertices.size()*sizeof(Vertex)
                                                options:MTLResourceStorageModeShared];
        id<MTLBuffer> ibuf = [device newBufferWithBytes:batch.indices.data()
                                                 length:batch.indices.size()*sizeof(uint32_t)
                                                options:MTLResourceStorageModeShared];

        [encoder setVertexBuffer:vbuf offset:0 atIndex:0];

        FragUniforms fu = { (batch.texture_id != 0) ? 1u : 0u, {0,0,0} };
        [encoder setFragmentBytes:&fu length:sizeof(fu) atIndex:0];

        if (batch.texture_id != 0) {
            id<MTLTexture> tex = nil;
            for (auto &it : textures) {
                if (it.second && it.second->id == batch.texture_id && it.second->mtlTexture) {
                    tex = (__bridge id<MTLTexture>)(it.second->mtlTexture);
                    break;
                }
            }

            [encoder setFragmentTexture:tex atIndex:0];
        } else {
            [encoder setFragmentTexture:nil atIndex:0];
        }

        [encoder setFragmentSamplerState:sampler atIndex:0];

        [encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                            indexCount:batch.indices.size()
                             indexType:MTLIndexTypeUInt32
                           indexBuffer:ibuf
                     indexBufferOffset:0];
    }

    [encoder endEncoding];
    [cmdBuffer presentDrawable:drawable];
    [cmdBuffer commit];

    }


    batches.clear();
    commands.clear();
}

void MetalRenderer::present() {

}

void MetalRenderer::resize_viewport(int w,int h) {
    Viewport[0] = w; Viewport[1] = h;
    if (metal_layer_) {
        CAMetalLayer* layer = (__bridge CAMetalLayer*)metal_layer_;
        layer.drawableSize = CGSizeMake(w, h);
    }
    projection = glm::ortho(0.0f, float(w), float(h), 0.0f, -1.0f, 1.0f);
}

void MetalRenderer::destroy() {

    if (pipeline_) {
        id<MTLRenderPipelineState> p = (__bridge_transfer id<MTLRenderPipelineState>)pipeline_;
        (void)p;
        pipeline_ = nullptr;
    }
    if (sampler_) {
        id<MTLSamplerState> s = (__bridge_transfer id<MTLSamplerState>)sampler_;
        (void)s;
        sampler_ = nullptr;
    }
    if (queue_) {
        id<MTLCommandQueue> q = (__bridge_transfer id<MTLCommandQueue>)queue_;
        (void)q;
        queue_ = nullptr;
    }

    for (auto it = textures.begin(); it != textures.end(); ++it) {
        if (it->second && it->second->mtlTexture) {
            id<MTLTexture> tex = (__bridge_transfer id<MTLTexture>)(it->second->mtlTexture);
            (void)tex;
            it->second->mtlTexture = nullptr;
        }
    }

    textures.clear();
    _texture_sizes.clear();

    if (metal_view_) {
        SDL_Metal_DestroyView(metal_view_);
        metal_view_ = 0;
    }

    device_ = nullptr;
    metal_layer_ = nullptr;
}

void MetalRenderer::unload_texture(Uint32 tex_id) {
    if (tex_id == 0) return;
    for (auto it = textures.begin(); it != textures.end(); ++it) {
        if (it->second && it->second->id == tex_id) {
            if (it->second->mtlTexture) {
                id<MTLTexture> tex = (__bridge_transfer id<MTLTexture>)it->second->mtlTexture;
                (void)tex;
                it->second->mtlTexture = nullptr;
            }
            _texture_sizes.erase(tex_id);
            textures.erase(it);
            break;
        }
    }
}

void MetalRenderer::set_context(const void* ctx) {}

void* MetalRenderer::get_context() { return device_; }

// --- stubs ---
bool MetalRenderer::load_font(const std::string&, const std::string&, int) { return false; }
std::shared_ptr<Texture> MetalRenderer::get_texture(const std::string& path) {
    auto it = textures.find(path);
    if (it != textures.end()) return it->second;
    return load_texture(path);
}
void MetalRenderer::unload_font(const Font&) {}
void MetalRenderer::draw_text(const std::string&, float, float, float, float, const glm::vec4&,
                              const std::string&, int, const UberShader&, int) {}
void MetalRenderer::draw_line(float, float, float, float, float, float, const glm::vec4&, int) {}
void MetalRenderer::draw_triangle(float, float, float, float, float, float,
                                  float, const glm::vec4&, bool, int) {}
void MetalRenderer::draw_circle(float, float, float, float, const glm::vec4&, bool, int, int) {}
void MetalRenderer::draw_polygon(const std::vector<glm::vec2>&, float, const glm::vec4&, bool, int) {}
void MetalRenderer::_render_command(const DrawCommand&) {}
void MetalRenderer::_set_default_font(const std::string&) {}
void MetalRenderer::_render_fbo() {}
void MetalRenderer::_set_effect_uniforms(const UberShader&, const glm::vec2&) {}
glm::vec2 MetalRenderer::_get_texture_size(Uint32) const { return {0,0}; }
