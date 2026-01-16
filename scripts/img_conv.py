import sys
from PIL import Image
import os

def convert_to_header(input_path, output_path, var_name):
    img = Image.open(input_path).convert('RGBA')
    width, height = img.size
    data = list(img.getdata())
    
    with open(output_path, 'w') as f:
        f.write(f"#ifndef {var_name.upper()}_H\n")
        f.write(f"#define {var_name.upper()}_H\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"const uint32_t {var_name}_width = {width};\n")
        f.write(f"const uint32_t {var_name}_height = {height};\n")
        f.write(f"const uint32_t {var_name}_data[] = {{\n")
        
        # Write pixels in 0xRRGGBBAA format (or whatever the kernel expects)
        # ParadoxOS uses color_t which seems to be 0xAARRGGBB or similar. 
        # Let's use 0xAARRGGBB to match the kernel.
        for r, g, b, a in data:
            color = (a << 24) | (r << 16) | (g << 8) | b
            f.write(f"0x{color:08X}, ")
            
        f.write("\n};\n\n")
        f.write(f"#endif\n")

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: python img_conv.py <input.png> <output.h> <var_name>")
    else:
        convert_to_header(sys.argv[1], sys.argv[2], sys.argv[3])
