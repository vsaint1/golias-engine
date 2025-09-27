import re
import sys

def parse_sdl_enum_to_lua(input_file, output_file, table_name="SDL"):
    with open(input_file, "r") as f:
        content = f.read()

    # Remove multi-line comments
    content = re.sub(r"/\*.*?\*/", "", content, flags=re.DOTALL)
    # Remove single-line comments
    content = re.sub(r"//.*", "", content)
    # Extract the enum block
    match = re.search(r"typedef\s+enum\s+\w*\s*{(.*?)}\s*\w+;", content, re.DOTALL)
    if not match:
        print("No enum found in file")
        return

    enum_body = match.group(1)
    lines = [l.strip() for l in enum_body.split(",") if l.strip()]
    lua_lines = [f"local {table_name} = {{}}"]

    value_map = {}
    counter = 0

    for line in lines:
        if not line:
            continue
        # Split name = value
        if "=" in line:
            name, value = line.split("=", 1)
            name = name.strip()
            value = value.strip()
            # Handle alias to another constant
            if re.match(r"^[A-Za-z_][A-Za-z0-9_]*$", value):
                resolved_value = value_map.get(value, value)
                value_map[name] = resolved_value
            else:
                # Convert hex/decimal to int
                try:
                    resolved_value = int(value, 0)
                except ValueError:
                    resolved_value = value
                value_map[name] = resolved_value
                counter = int(resolved_value) + 1
        else:
            name = line
            value_map[name] = counter
            counter += 1

        lua_lines.append(f"{table_name}.{name} = {value_map[name]}")

    lua_lines.append(f"\nreturn {table_name}")

    with open(output_file, "w") as f:
        f.write("\n".join(str(l) for l in lua_lines))

    print(f"Lua constants written to {output_file}")


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python enum_to_lua.py input_header.h output.lua")
    else:
        parse_sdl_enum_to_lua(sys.argv[1], sys.argv[2])
