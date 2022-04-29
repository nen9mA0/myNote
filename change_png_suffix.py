import os

def TraverseDir(path):
    old_dir = os.getcwd()
    os.chdir(path)
    dir_list = os.listdir()
    for file in dir_list:
        new_path = os.path.join(path, file)
        if os.path.isdir(new_path):
            TraverseDir(new_path)
        else:
            if file.endswith(".PNG"):
                old = os.path.join(path, file)
                new_file = file[:-4] + ".png"
                new = os.path.join(path, new_file)
                print("rename %s to %s" %(old, new))
                os.rename(old, new)
    os.chdir(old_dir)


TraverseDir(os.getcwd())

