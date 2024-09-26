import bpy
import sys
import os

def main():
    # Get command-line arguments after '--'
    argv = sys.argv
    if '--' in argv:
        argv = argv[argv.index('--') + 1:]
    else:
        argv = []

    if len(argv) < 2:
        print("Usage: blender --background --python extract_textures.py -- <obj_file_path> <output_directory>")
        return

    obj_file_path = argv[0]
    output_dir = argv[1]

    if not os.path.exists(obj_file_path):
        print(f"Error: OBJ file does not exist: {obj_file_path}")
        return

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Clear existing data
    bpy.ops.wm.read_factory_settings(use_empty=True)

    # Import the OBJ file
    bpy.ops.import_scene.obj(filepath=obj_file_path)

    # Keep track of saved images to avoid duplicates
    saved_images = set()

    # Iterate over all materials in the scene
    for mat in bpy.data.materials:
        if mat.use_nodes:
            for node in mat.node_tree.nodes:
                if node.type == 'TEX_IMAGE':
                    image = node.image
                    if image and image.name not in saved_images:
                        # Define the output file path
                        image_filepath = os.path.join(output_dir, f"{image.name}.png")
                        # Set the image's file path and format
                        image.filepath_raw = image_filepath
                        image.file_format = 'PNG'
                        # Save the image
                        image.save()
                        print(f"Saved image: {image_filepath}")
                        saved_images.add(image.name)

if __name__ == "__main__":
    main()
