import re
import subprocess
from pylatex import UnsafeCommand, Document, Command, PageStyle, Head, MiniPage, LargeText, LineBreak, MediumText, LongTabu, NewPage, Package, Section, Description, Figure
from pylatex.utils import NoEscape, bold
import datetime
import os


def get_evac_time(testnumber):

    log_name = "./test_" + str(testnumber) + "/log_test_" + str(testnumber) + ".txt"

    with open(log_name) as log:
        evac_time = []
        for line in log:
            if re.match(r'^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2},\d{3} - INFO - evac_time: \d{2}.\d{6} <= \d{2}.\d{6} <= \d{2}.\d{6}$', line):
                evac_time.append(float(re.split('<=', line)[-2]))
            else:
                continue

    return evac_time


def generate_eva_report():

    geometry_options = {
    "head": "40pt",
    "margin": "0.5in",
    "bottom": "0.6in",
    "includeheadfoot": True
    }
    doc = Document(geometry_options=geometry_options)

    reportStyle = PageStyle("reportStyle")

    with reportStyle.create(Head("R")) as left_header:
        with left_header.create(MiniPage(width=NoEscape(r"0.49\textwidth"),pos='c', align='l')) as title_wrapper:
            title_wrapper.append(LargeText(bold("RiMEA-Projekt")))
            title_wrapper.append(LineBreak())
            title_wrapper.append(MediumText(bold("Anlyse")))

    doc.preamble.append(reportStyle)

    doc.change_document_style("reportStyle")

    with doc.create(LongTabu("X[c] X[c]",row_height=1.5)) as report_table:
        report_table.add_row(["Test","evacuation time(s)"],mapper=bold)
        report_table.add_hline()

    for i in range(1,4):
        report_table.add_row(i, get_evac_time(i)[0])

    doc.append(NewPage())

    doc.generate_pdf("RiMEA-Projekt-Evacution-Analyse", clean_tex=False)


def generate_info_report():

    geometry_options = {
            "head": "40pt",
            "margin": "0.5in",
            "bottom": "0.6in",
            "includeheadfoot": True
        }

    doc = Document(documentclass='article', geometry_options=geometry_options)

    generate_cover2(doc)
    doc.append(NewPage())

    generate_status_tabel(doc)
    doc.append(NewPage())

    for i in range(1, 16):
        get_test_description(doc, i)
        doc.append(NewPage())
        generate_info_list(doc, i)
        get_diagrams(doc, i)

    doc.generate_pdf('RiMEA-Projekt-Analyse', clean_tex=False)


def generate_cover(doc):

    doc.preamble.append(Command('title', 'RiMEA-Projekt Analyse'))
    doc.preamble.append(Command('author', get_git_status()[1]))
    doc.preamble.append(Command('date', get_git_status()[2]))

    doc.packages.append(Package('titling'))

    branch = r"\begin{center}Branch: "+ get_git_status()[0] + "\par"
    doc.preamble.append(Command('predate', NoEscape(branch)))

    commit = r"\par commit: " + get_git_status()[3] + "\par\end{center}"
    doc.preamble.append(Command('postdate', NoEscape(commit)))

    doc.append(NoEscape(r'\maketitle'))


def generate_cover2(doc):

	doc.append(NoEscape(r"\begin{titlepage}"))
	doc.append(NoEscape(r"\begin{center}"))

	with doc.create(Figure(position='t')) as logo:
		logo.add_image("../../forms/jupedsim.png")

	doc.append(NoEscape(r"\textsc{\LARGE J\"ulich Pedestrian Simulator}\\[1.5cm]"))
	doc.append(NoEscape(r"\textsc{\small Forschungszentrum J\"ulich GmbH}\\[0.5cm]"))

	doc.append(NoEscape(r"\hrule \vspace{1.5mm}"))
	doc.append(NoEscape(r"{ \huge \bfseries RiMEA-Projekt Analyse Report}\\[0.4cm]"))
	doc.append(NoEscape(r"\hrule \vspace{5.5cm}"))

	author = get_git_status()[1] + "\par"
	date = get_git_status()[2] + "\par"
	branch = "Branch: " + get_git_status()[0] + "\par"
	commit = "Commit: " + get_git_status()[3] + "\par"

	InfoBlock = MiniPage(width=NoEscape(r"1\textwidth"),align='c')
	today = "Date: "+ datetime.datetime.now().strftime("%d - %m - %Y   (%H:%M)") + "\par"

	InfoBlock.append(NoEscape(today))
	InfoBlock.append(NoEscape(branch))
	InfoBlock.append(NoEscape(commit))

	doc.append(InfoBlock)

	doc.append(NoEscape(r"\end{center}"))
	doc.append(NoEscape(r"\end{titlepage}"))


def generate_status_tabel(doc):

    with doc.create(LongTabu("X[c] X[c] X[r]")) as status_table:
        header_row1 = ["Test Number", "Status", "Exec time"]

        status_table.add_row(header_row1, mapper=[bold])
        status_table.add_hline()
        status_table.add_empty_row()
        status_table.end_table_header()

        for i in range(15):
            row = [str(i+1), get_tests_status(i+1),get_exec_time(i+1)+'[sec]']
            status_table.add_row(row)
            get_evac_time(i+1)


def generate_info_table(doc):

    with doc.create(LongTabu("X[l] X[r]")) as info_table:
        header_row1 = ["Test Number", "Info"]

        info_table.add_row(header_row1, mapper=[bold])
        info_table.add_hline()
        info_table.add_empty_row()
        info_table.end_table_header()

        for i in range(3):
            row_1 = [str(i + 1), get_log(i + 1)[0]]
            row_2 = [" ", get_log(i + 1)[1]]
            info_table.add_row(row_1)
            info_table.add_empty_row()
            info_table.add_row(row_2)
            info_table.add_hline()
            info_table.add_empty_row()


def generate_info_list(doc, testnumber):

    section_name = 'Test {} results'.format(testnumber)

    with doc.create(Section(section_name)):
        with doc.create(Description()) as desc:
            desc.add_item("\t", get_log(testnumber)[0])
            desc.add_item("\t", get_log(testnumber)[1])


def get_git_status():

    branch =str(subprocess.check_output(["git", "status"]))
    branch = branch.strip("b'On branch")
    branch = branch.split("\\n")[0]

    git_status = str(subprocess.check_output(["git", "show", "--pretty=medium"]))
    git_status = git_status.split("\\n")
    commit = git_status[0].split(' ')[1]
    author = git_status[1]
    date = git_status[2]

    return branch, author, date, commit


def get_tests_status(testnumber):

    log_name = "./test_" + str(testnumber) + "/log_test_" + str(testnumber) + ".txt"

    with open(log_name) as log:
        for line in log:
            if re.match(r'^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2},\d{3}.*?INFO.*?runtest_rimea.*?SUCCESS$',line):
                status = "Succeed"
                break

            else:
                status = "Failed"

    return status


def get_log(testnumber):

    log_name = "./test_" + str(testnumber) + "/log_test_" + str(testnumber) + ".txt"
    with open(log_name) as log:
        return log.read().split("\n")[-3:-1]


def get_diagrams(doc, testnumber):
    test_directory = os.path.join(os.getcwd(), "test_{}".format(testnumber))
    is_diagram = False

    for filename in os.listdir(test_directory):
        if filename.endswith(".png"):
            is_diagram = True
            diagram_path = '{}/test_{}/{}'.format(os.getcwd(),testnumber,filename)
            diagram_description = diagram_path.split("/")[-1]
    if is_diagram:
        with doc.create(Figure(position='h!')) as diagram:
            diagram.add_image(diagram_path)
            diagram.add_caption(diagram_description)


def get_exec_time(testnumber):
    start_direc = os.getcwd()
    direc = start_direc + '/test_' + str(testnumber)
    os.chdir(direc)
    textfile=open('log_test_' + str(testnumber) + '.txt')
    for line in textfile:
        line=line.split(' ')
        exec_time='-'
        if line[0] == 'Execution':
            exec_time=line[2]
    os.chdir(start_direc)
    return exec_time

def get_test_description(doc, testnumber):
	doc.append(NewPage())
	description = UnsafeCommand('input','test_description/test_{}.tex'.format(testnumber))
	doc.append(description)

def test():
    print('reportscript successfully imported')


if __name__ == "__main__":
		generate_info_report()
		print('reportscript successfully created')
