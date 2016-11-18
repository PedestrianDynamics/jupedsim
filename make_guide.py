#======================================
# convert _posts/*.md to _tex/*.tex   #
# mkdir _tex/ if it does not exit.    #
#======================================
import glob
import os
import re
import shlex
import subprocess
import shutil

def clean_file(filename):
    """
    Remove yaml header and fix path to images (remove {{ site.baseurl }})
    """

    f =  open(filename, "r")
    lines = f.readlines()
    if not len(lines):
        print "WARNING: can not read file ", filename
        raw_input("pause")
        
    add_line = 0  # condition when to start adding new lines
    newlines = [] # new lines will be written in filename

    for line in lines:
        # look for first appearance of "---"
        if not add_line and line.startswith("---"):
            add_line = 1
            continue # ignore this line

        if add_line:
            # fix image urls. Replace "{{ base.url }}" with pwd.
            line = re.sub(r"\{\{\s* site.baseurl \s*\}\}", os.getcwd(), line)
            newlines.append(line)
            # gif --> png?

    # write new content in file
    f = open(filename, "w")
    f.write("".join(newlines))
    f.close()



if __name__ == "__main__":

    if not os.path.exists("_tex"):
        os.makedirs("_tex")
        
    markdown_files = glob.glob("_posts/*.md")
    latex_files = []
    for markdown_file in markdown_files:
        print "<< ", markdown_file
        latex_file = os.path.join("_tex", os.path.basename(os.path.splitext(markdown_file)[0]) ) + ".tex"
        latex_files.append(latex_file)
        print ">> ", latex_file
        cmd = shlex.split("kramdown --input GFM %s --no-auto-ids --output latex" % markdown_file)
        # markdown to latex
        with open(latex_file, 'w') as fout:
            proc = subprocess.Popen(cmd, stdout=fout)

        proc.wait()
        # remove yaml-header from tex files and fix images
        clean_file(latex_file)

    # produce jupedsim.tex
    subprocess.Popen("make")
