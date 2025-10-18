import os, sys, zlib, struct

def pack(folder, out_file):
    files = []
    for root, _, fs in os.walk(folder):
        for f in fs:
            path = os.path.join(root, f)
            rel = os.path.relpath(path, folder).replace("\\", "/")
            files.append(rel)

    with open(out_file, "wb") as out:
        out.write(struct.pack("<I", len(files)))  # file_count
        for rel in files:
            path = os.path.join(folder, rel)
            data = open(path, "rb").read()
            comp = zlib.compress(data)
            out.write(struct.pack("<I", len(rel))) # path_length
            out.write(rel.encode())
            out.write(struct.pack("<II", len(comp), len(data))) # comp_size, orig_size
            out.write(comp)

    print(f"✅ Packed {len(files)} files into {out_file}")
    print(f"✅ Source folder: {folder}")
    print(f"✅ Output file: {out_file}")

if __name__ == "__main__":
    pack(sys.argv[1], sys.argv[2])