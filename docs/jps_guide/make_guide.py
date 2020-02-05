#======================================
# 1. convert pages/*.md to _tex/*.tex
# 2. cp post-checkout in  ../../.git/hooks/post-checkout
# 3. calls make --> creates pdf file
# 4. calls make clean
# mkdir _tex/ if it does not exit.
# _tex will be deleted at the end of the script with make clean
# dependencies:
# - kramdown
# - latex
# - make
#======================================
import sys
import glob
import os
import re
import shlex
import shutil
import subprocess
from shutil import copyfile

def clean_file(filename, img_dir):
    """
    Remove yaml header and fix path to images (remove {{ site.baseurl }})
    """

    f =  open(filename, "r")
    lines = f.readlines()
    if not lines:
        print("WARNING: can not read file ", filename)
        input("pause")

    add_line = 0  # condition when to start adding new lines
    newlines = [] # new lines will be written in filename

    for line in lines:
        # look for first and second appearance of "---"
        # ---
        # \end{itemize}
         # the last itemize  is stange indeed.
        if add_line == 0 and line.startswith("---"):
            add_line += 1
            continue # ignore this line

        if add_line == 1:
            add_line += 1
            continue # ignore following line.

        if add_line == 2:
            # fix image urls. Replace "{{ base.url }}" with pwd.
            line = re.sub(r"\{\{\s* site.baseurl \s*\}\}", img_dir, line)
            newlines.append(line)
            # gif --> png?

    # write new content in file
    f = open(filename, "w")
    f.write("".join(newlines))
    f.close()


if __name__ == "__main__":
    pathname = os.path.dirname(sys.argv[0])
    if not os.path.exists("_tex"):
        os.makedirs("_tex")
        print("make _tex")

    for module in ["jupedsim", "jpscore", "jpsreport"]:
        if not os.path.exists(os.path.join("_tex", module)):
            os.makedirs(os.path.join("_tex", module))
            print(os.path.join("_tex", module))

        markdown_files = glob.glob("../pages/%s/*.md" % module)
        img_dir = ".."
        if shutil.which("kramdown") is None:
            sys.exit("error: kramdown not found.")

        if shutil.which("xelatex") is None:
            sys.exit("error: xelatex not found.")

        for markdown_file in markdown_files:
            latex_file = os.path.join("_tex", module, os.path.basename(os.path.splitext(markdown_file)[0]) ) + ".tex"
            print("markdown_file %s " % markdown_file)
            print(">> %s" % latex_file)
            cmd = shlex.split("kramdown --input GFM %s --no-auto-ids --output latex" % markdown_file)
            # markdown to latex
            with open(latex_file, 'w') as fout:
                proc = subprocess.Popen(cmd, stdout=fout)

            proc.wait()
            # remove yaml-header from tex files and fix images
            clean_file(latex_file, img_dir)


    source_path = os.path.normpath(pathname + os.sep + os.pardir + os.sep + os.pardir)
    if not os.path.exists("./titlepage.tex"):
        sys.exit("could not find titlepage.tex. git checkout was not successful!")

    # process jupedsim.tex
    subprocess.call("make")
    res = subprocess.call(shlex.split("make clean"))


