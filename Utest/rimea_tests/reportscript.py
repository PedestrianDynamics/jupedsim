import os
import re
from pylatex import Document, PageStyle, Head, Foot, MiniPage, \
    StandAloneGraphic, MultiColumn, Tabu, LongTabu, LargeText, MediumText, \
    LineBreak, NewPage, Tabularx, TextColor, simple_page_number
from pylatex.utils import bold, NoEscape


## Import a number of test(int), return all evac time in the log of this test
def get_evac_time(testnumber):
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

	return evac_time


def generate_report():
	geometry_options = {
		"head": "40pt",
		"margin": "0.5in",
		"bottom": "0.6in",
		"includeheadfoot": True
	}
	doc = Document(geometry_options=geometry_options)

	first_page = PageStyle("firstpage")

	with first_page.create(Head("L")) as left_header:
		with left_header.create(MiniPage(width=NoEscape(r"0.49\textwidth"),
		                                  pos='c', align='l')) as title_wrapper:
			title_wrapper.append(LargeText(bold("RiMEA-Projekt")))
			title_wrapper.append(LineBreak())
			title_wrapper.append(MediumText(bold("Anlyse")))

	doc.preamble.append(first_page)
	doc.change_document_style("firstpage")
	## Add table
	with doc.create(LongTabu("X[c] X[c]",
	                         row_height=1.5)) as report_table:
		report_table.add_row(["Test",
		                    "evacuation time(s)"],
		                   mapper=bold)
		report_table.add_hline()
		for i in range(1,4):  # TODO: Automatize filling the content
			report_table.add_row(i, get_evac_time(i)[0])  # TODO: Calculate mean

	doc.append(NewPage())

	doc.generate_pdf("RiMEA-Projekt Analyse", clean_tex=False)

if __name__ == '__main__':
	generate_report()