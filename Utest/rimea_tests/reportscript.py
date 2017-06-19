import os
import re, sys, subprocess
from pylatex import Document, Command, PageStyle, Head, MiniPage, LargeText, LineBreak, \
	LineBreak, MediumText, LongTabu, NewPage, Package, Center, Section
from pylatex.utils import NoEscape, bold, verbatim
from pylatex.base_classes import Environment


<<<<<<< HEAD

def get_evac_time(testnumber):
		"""
		Import a number of test
		return all evac time in the log of this test
		:param testnumber: int, the number of test
		:return: list, contains all evacution times in a log txt
		"""
		log_name = "./test_" + str(testnumber) + "/log_test_" + str(testnumber) + ".txt"

		evac_time = []
		for line in log_name:
=======
 
def get_evac_time(testnumber):
    """
    Import a number of test
    return all evac time in the log of this test
    :param testnumber: int, the number of test
    :return: list, contains all evacution times in a log txt
    """
    log_name = "./test_" + str(testnumber) + "/log_test_" + str(testnumber) + ".txt"

	with open(log_name) as log:
	    evac_time = []
		for line in log:
>>>>>>> f1d5b18ed06d758f7d633736ec2665799ecb1fda
			## Use Regular Expression to filter, fetch lines which contain the evac time.
			if re.match(
					r'^2017-\d{2}-\d{2} \d{2}:\d{2}:\d{2},\d{3} - INFO - evac_time: \d{2}.\d{6} <= \d{2}.\d{6} <= \d{2}.\d{6}$',
					line):
				evac_time.append(float(re.split('<=', line)[-2])) ## The second to last number is the evac time

			else:
				continue

<<<<<<< HEAD
		return evac_time ## even only last result


def generate_eva_report():
		"""
	  generate a report which contains the evacution time for every test
	  """
		geometry_options = {
			"head": "40pt",
			"margin": "0.5in",
			"bottom": "0.6in",
			"includeheadfoot": True
		}
		doc = Document(geometry_options=geometry_options)
=======
	return evac_time ## even only last result


def generate_eva_report():
    """
    generate a report which contains the evacution time for every test
    """
	geometry_options = {
		"head": "40pt",
		"margin": "0.5in",
		"bottom": "0.6in",
		"includeheadfoot": True
	}
	doc = Document(geometry_options=geometry_options)
>>>>>>> f1d5b18ed06d758f7d633736ec2665799ecb1fda

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

			for i in range(1,4):
				report_table.add_row(i, get_evac_time(i)[0])

		doc.append(NewPage())

		doc.generate_pdf("RiMEA-Projekt Evacution Analyse", clean_tex=False)


def generate_info_report():
	"""
<<<<<<< HEAD
	generate a report with cover, status und last 2 lines info for every test
=======
	generate a report with cover, status und last 2 lines infos for every test
>>>>>>> f1d5b18ed06d758f7d633736ec2665799ecb1fda
	"""
	geometry_options = {
			"head": "40pt",
			"margin": "0.5in",
			"bottom": "0.6in",
			"includeheadfoot": True
		}

	doc = Document(documentclass='report', geometry_options=geometry_options)

	## add preamble part
	generate_cover(doc)


	doc.generate_pdf("RiMEA-Projekt Analyse", clean_tex=False)



def generate_cover(doc):
	"""
	generate a cover for generate_info_report func
	cover contains name, date and branch info
	:param doc: a Document Class instance
	:return: null
	"""

	doc.preamble.append(Command('title', 'RiMEA-Projekt Analyse'))
	doc.preamble.append(Command('author', get_git_status()[1]))
	doc.preamble.append(Command('date', get_git_status()[2]))

	doc.packages.append(Package('titling'))
	branch = r"\begin{center}Branch: "+ get_git_status()[0] + "\end{center}"
	doc.preamble.append(Command('postdate', NoEscape(branch)))

	doc.append(NoEscape(r'\maketitle'))

def get_git_status():
	branch = subprocess.check_output(["git", "status"]).splitlines()[0].split(' ')[-1]

	git_status = subprocess.check_output(["git", "show", "--pretty=medium"])
	author = git_status.split('\n')[1]
	date = git_status.split('\n')[2]

	return branch, author, date

if __name__ == "__main__":
		generate_info_report()
	# generate_eva_report()



