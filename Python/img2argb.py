from PIL import Image

def resize_image(image_path, width, height):
    # Open the image file
    image = Image.open(image_path)
    
    # Resize the image
    resized_image = image.resize((width, height))
    
    # Convert the image to RGBA mode
    rgba_image = resized_image.convert("RGBA")
    
    # Get the pixel data as a flat list of tuples (R, G, B, A)
    pixel_data = list(rgba_image.getdata())
    
    # Create the ARGB 2D array
    argb_array = []
    for i in range(height):
        row = []
        for j in range(width):
            pixel = pixel_data[i * width + j]
            argb_value = ((0xFF << 24) | (pixel[0] << 16) | (pixel[1] << 8) | pixel[2])
            row.append(argb_value)
        argb_array.append(row)
    
    return argb_array

def save_resized_image(image_path, output_path, width, height):
    # Resize the image
    resized_image = Image.open(image_path).resize((width, height))
    
    # Save the resized image
    resized_image.save(output_path)

# Example usage
image_path = "rose.jpg"
output_path = "resized_image.jpg"
width = 272
height = 272

# Resize and save the image
save_resized_image(image_path, output_path, width, height)

# Generate the ARGB array
argb_array = resize_image(output_path, width, height)

# Write the ARGB array to a text file
with open("argb_array.txt", "w") as file:
    for row in argb_array:
        file.write("{" + ", ".join(str(value) for value in row) + "},\n")
