import numpy as np
from PIL import Image
import matplotlib.pyplot as plt
import sys

def convert_bmp_to_rgb332(image_path, image_name, rotate_k):
    """Convert BMP to RGB332 format and verify the conversion."""
    # Load the image
    img = Image.open(image_path)
    print(f"Original image size: {img.size}, mode: {img.mode}")
    
    # Convert to RGB if not already
    if img.mode != 'RGB':
        img = img.convert('RGB')
    
    # Get image as numpy array
    img_array = np.array(img)
    img_array = np.rot90(img_array, k=rotate_k)
    height, width, _ = img_array.shape
    
    # Convert to RGB332 (1 byte per pixel)
    # 3 bits for R (0-7), 3 bits for G (0-7), 2 bits for B (0-3)
    r = (img_array[:,:,0] >> 5) & 0x07  # Extract top 3 bits
    g = (img_array[:,:,1] >> 5) & 0x07  # Extract top 3 bits
    b = (img_array[:,:,2] >> 6) & 0x03  # Extract top 2 bits
    
    # Pack into a single byte per pixel: RRRGGGBB
    rgb332 = (r << 5) | (g << 2) | b
    
    # Flatten the array for C export
    flat_rgb332 = rgb332.flatten()

    #Save array in a text file
    txt_filename = f"{image_name}_array.txt"
    with open(txt_filename, "w") as txt_file:
        txt_file.write(f"# RGB332 image data ({width}x{height})\n")
        txt_file.write(f"# Width: {width}\n")
        txt_file.write(f"# Height: {height}\n")
        txt_file.write(f"# Format: One byte per pixel, RRRGGGBB\n")
        txt_file.write("#\n")
        txt_file.write("# Array values (decimal):\n")
        rgb_mat = np.reshape(rgb332, (height, width))
        for row in rgb_mat:
            txt_file.write(" ".join([f"{val:3d}" for val in row]) + "\n")
    
    print(f"Saved array data to {txt_filename}")
    
    # Create header file
    filename = image_name+".h"
    with open(filename, "w") as f:
        f.write("#include <Arduino.h>\n")   
        f.write(f"// RGB332 image data ({width}x{height})\n")
        f.write(f"#define {image_name}_WIDTH {width}\n")
        f.write(f"#define {image_name}_HEIGHT {height}\n")
        f.write("const uint8_t "+image_name+"[] = {\n")
        
        # Write data in rows of 16 values
        for i in range(0, len(flat_rgb332), 16):
            row = flat_rgb332[i:i+16]
            f.write("    " + ", ".join([f"0x{val:02X}" for val in row]))
            if i + 16 < len(flat_rgb332):
                f.write(",")
            f.write("\n")
        f.write("};\n")
    
    print(f"Saved C array to image_rgb332.h")
    
    # Reconstruct image for validation
    # Convert RGB332 back to RGB888
    # Better scaling that preserves brightness
    r_recon = np.uint8(((rgb332 >> 5) & 0x07) * (255.0 / 7.0))
    g_recon = np.uint8(((rgb332 >> 2) & 0x07) * (255.0 / 7.0))
    b_recon = np.uint8((rgb332 & 0x03) * (255.0 / 3.0))
    
    recon_array = np.zeros((height, width, 3), dtype=np.uint8)
    recon_array[:,:,0] = r_recon
    recon_array[:,:,1] = g_recon
    recon_array[:,:,2] = b_recon
    
    # Convert back to PIL Image
    recon_img = Image.fromarray(recon_array)
    
    # Display both images side by side
    plt.figure(figsize=(12, 6))
    plt.subplot(1, 2, 1)
    plt.title("Original Image")
    plt.imshow(img)
    plt.axis('off')
    
    plt.subplot(1, 2, 2)
    plt.title("Reconstructed Image (RGB332)")
    plt.imshow(recon_img)
    plt.axis('off')
    
    plt.tight_layout()
    plt.show()
    
    return flat_rgb332, (width, height)

if __name__ == "__main__":
    # Get image path from command line or use default
    rotate = 3
    extension = ".jpeg"
    image_name = "kar" 
    image_path = f"{image_name}{extension}" 
    convert_bmp_to_rgb332(image_path, image_name, rotate)