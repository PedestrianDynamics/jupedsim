import re, subprocess
from pylatex import Document, Command, PageStyle, Head, MiniPage, LargeText, LineBreak, \
		MediumText, LongTabu, NewPage, Package, Section,  Description, NoEscape, Subsection, Figure, Center
from pylatex.utils import NoEscape, bold, verbatim
from pylatex.base_classes import Environment



def get_evac_time(testnumber):
		"""
		Fetch the evacuation time from log.txt of tests
		:param testnumber: the number of test(int)
		:return: All evacuation times(list)
		"""
		log_name = "./test_" + str(testnumber) + "/log_test_" + str(testnumber) + ".txt"

		with open(log_name) as log:
				evac_time = []
				for line in log:
					## Use Regular Expression to filter, fetch lines which contain the evac time.
					if re.match(
							r'^2017-\d{2}-\d{2} \d{2}:\d{2}:\d{2},\d{3} - INFO - evac_time: \d{2}.\d{6} <= \d{2}.\d{6} <= \d{2}.\d{6}$',
							line):
						evac_time.append(float(re.split('<=', line)[-2])) ## The second to last number is the evac time

					else:
						continue

		return evac_time ## even only last result


def generate_eva_report():
		"""
    Generate a report which contains the evacution time for every test
    """
		geometry_options = {
		"head": "40pt",
		"margin": "0.5in",
		"bottom": "0.6in",
		"includeheadfoot": True
		}
		doc = Document(geometry_options=geometry_options)

		## Define style of report
		reportStyle = PageStyle("reportStyle")

		with reportStyle.create(Head("R")) as left_header:
				with left_header.create(MiniPage(width=NoEscape(r"0.49\textwidth"),
				                                  pos='c', align='l')) as title_wrapper:
					title_wrapper.append(LargeText(bold("RiMEA-Projekt")))
					title_wrapper.append(LineBreak())
					title_wrapper.append(MediumText(bold("Anlyse")))

		doc.preamble.append(reportStyle)

		## Apply Pagestyle
		doc.change_document_style("reportStyle")

		## Create table
		with doc.create(LongTabu("X[c] X[c]",
		                       row_height=1.5)) as report_table:
				report_table.add_row(["Test",
				                    "evacuation time(s)"],
				                   mapper=bold)
				report_table.add_hline()

		## Write the results of 3 tests in table
		for i in range(1,4):
			report_table.add_row(i, get_evac_time(i)[0])

		doc.append(NewPage())

		doc.generate_pdf("RiMEA-Projekt Evacution Analyse", clean_tex=False)


def generate_info_report():
			"""
			Generate a report with cover, status und last 2 lines in log for every test
			"""

			## Define the geometry for LaTeX files
			geometry_options = {
					"head": "40pt",
					"margin": "0.5in",
					"bottom": "0.6in",
					"includeheadfoot": True
				}


			## Create the LaTeX object, a instance of Document Class
			doc = Document(documentclass='article', geometry_options=geometry_options)

			## Add cover
			generate_cover2(doc)
			doc.append(NewPage())

			## Add status table
			generate_status_tabel(doc)
			doc.append(NewPage())

			## Add last 2 lines in log.txt
			for i in range(1, 4):
					generate_info_list(doc, i)


			doc.generate_pdf("RiMEA-Projekt Analyse", clean_tex=False)


def generate_cover(doc):
		"""
		Generate a cover for generate_info_report func
		Cover contains name, date and branch info
		:param doc: a Document Class instance
		:return: null
		"""

		## Convert in default command of LaTeX to make title
		## \title{}
		## \author{}
		## \date{}
		doc.preamble.append(Command('title', 'RiMEA-Projekt Analyse'))
		doc.preamble.append(Command('author', get_git_status()[1]))
		doc.preamble.append(Command('date', get_git_status()[2]))

		## Use titling package to add line on title
		doc.packages.append(Package('titling'))

		branch = r"\begin{center}Branch: "+ get_git_status()[0] + "\par"
		doc.preamble.append(Command('predate', NoEscape(branch)))

		commit = r"\par commit: " + get_git_status()[3] + "\par\end{center}"
		doc.preamble.append(Command('postdate', NoEscape(commit)))

		doc.append(NoEscape(r'\maketitle'))


def generate_cover2(doc):
		"""
		Generate a cover for generate_info_report func
		Without \maketitle in LaTeX, but using titlepage environment
		:param doc: LaTeX object, a instance of Document Class
		:return: null
		"""

		## Define titlepage in title environment
		doc.append(NoEscape(r"\begin{titlepage}"))
		doc.append(NoEscape(r"\begin{center}"))

		with doc.create(Figure(position='t', width=NoEscape(r'0.4\textwidth'))) as logo:
				logo.add_image("./figures/logo.png")

		doc.append(NoEscape(r"\textsc{\LARGE J\"ulich Pedestrian Simulator}\\[1.5cm]"))
		doc.append(NoEscape(r"\textsc{\small Forschungszentrum J\"ulich GmbH}\\[0.5cm]"))

		doc.append(NoEscape(r"\HRule \\[0.4cm]"))
		doc.append(NoEscape(r"{ \huge \bfseries RiMEA-Projekt Analyse Report}\\[0.4cm]"))
		doc.append(NoEscape(r"\HRule \\[1.5cm]"))

		## Add author date branch commit in a miniPage on title
		author = get_git_status()[1] + "\par"
		date = get_git_status()[2] + "\par"
		branch = "Branch: " + get_git_status()[0] + "\par"
		commit = "Commit: " + get_git_status()[3] + "\par"

		InfoBlock = MiniPage(width=NoEscape(r"0.8\textwidth"),
                align='c')
		InfoBlock.append(NoEscape(author))
		InfoBlock.append(NoEscape(date))
		InfoBlock.append(NoEscape(branch))
		InfoBlock.append(NoEscape(commit))

		doc.append(InfoBlock)

		doc.append(NoEscape(r"\end{center}"))
		doc.append(NoEscape(r"\end{titlepage}"))


def generate_status_tabel(doc):
		"""
		Generate a table which contains status of tests
		:param doc: LaTeX object, a instance of Document Class
		:return: null
		"""
		## Create a long table object in LaTeX object
		with doc.create(LongTabu("X[c] X[c]")) as status_table:
				header_row1 = ["Test Number", "Status"]

				status_table.add_row(header_row1, mapper=[bold])
				status_table.add_hline()
				status_table.add_empty_row()
				status_table.end_table_header()


				for i in range(3):
						row = [str(i+1), get_tests_status(i+1)]
						status_table.add_row(row)


def generate_info_table(doc):
		"""
		Generate a table which contains last 2 line of tests from log.txt
		:param doc: LaTeX object, a instance of Document Class
		:return: null
		"""

		## Create a long table object in LaTeX object
		with doc.create(LongTabu("X[l] X[r]")) as info_table:
				header_row1 = ["Test Number", "Info"]

				info_table.add_row(header_row1, mapper=[bold])
				info_table.add_hline()
				info_table.add_empty_row()
				info_table.end_table_header()

				## Add last 2 lines
				for i in range(3):
						row_1 = [str(i + 1), get_log(i + 1)[0]]
						row_2 = [" ", get_log(i + 1)[1]]
						info_table.add_row(row_1)
						info_table.add_empty_row()
						info_table.add_row(row_2)
						info_table.add_hline()
						info_table.add_empty_row()


def generate_info_list(doc, testnumber):
		"""
		Generate a list which contains last 2 line of tests from log.txt
		:param doc: LaTeX object, a instance of Document Class
		:return: null
		"""
		section_name = 'Test' + str(testnumber) + ': Last 2 lines in log_test_' + str(testnumber ) + '.txt'

		## Create a long table object in LaTeX object
		with doc.create(Section(section_name)):
				with doc.create(Description()) as desc:
						desc.add_item("Second last line: ", get_log(testnumber)[0])
						desc.add_item("Last line: ", get_log(testnumber)[1])

		doc.append(NewPage())


def get_git_status():
		"""
		Fetch information of the last git commit.
		:return: branch, author, date, commit of the newst commit(tuple)
		"""
		## Get the branch name by `git status` command
		branch = subprocess.check_output(["git", "status"]).splitlines()[0].split(' ')[-1]

		## Get the commit, author und date name by `git show` command
		git_status = subprocess.check_output(["git", "show", "--pretty=medium"])
		commit = git_status.split("\n")[0].split(' ')[1]
		author = git_status.split('\n')[1]
		date = git_status.split('\n')[2]

		return branch, author, date, commit


def get_tests_status(testnumber):
		"""
		Judge the status(succeed or failed) of a test by log.txt
		:param testnumber: the number of test(int)
		:return: status(Succeed or failed)(int)
		"""
		log_name = "./test_" + str(testnumber) + "/log_test_" + str(testnumber) + ".txt"

		with open(log_name) as log:
				for line in log:
								## Use Regular Expression to filter.
								if re.match(
										r'^2017-\d{2}-\d{2} \d{2}:\d{2}:\d{2},\d{3} - INFO - runtest_rimea_\d{1}.py exits with SUCCESS$',
										line):
										status = "Succeed"
										break

								else:
										status = "Failed"
 
		return status


def get_log(testnumber):
		"""
		Generate a path of a log.txt file
		Just for debugging
		:param testnumber: the number of test(int)
		:return: a path(str)
		"""
		log_name = "./test_" + str(testnumber) + "/log_test_" + str(testnumber) + ".txt"
		with open(log_name) as log:
				return log.read().split("\n")[-3:-1]


if __name__ == "__main__":
		generate_info_report()
		# generate_eva_report()


