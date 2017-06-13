import re

## Import a number of test(int), return all evac time in the log of this test
def get_evac_time(testnumber):
	log_name = "./test_" + str(testnumber) + "/log_test_" + str(testnumber) + ".txt"

	log = open(log_name)
	evac_time = []
	for line in log:
		## Use Regular Expression to filter, fetch lines which contain the evac time.
		if re.match(
				r'^2017-\d{2}-\d{2} \d{2}:\d{2}:\d{2},\d{3} - INFO - evac_time: \d{2}.\d{6} <= \d{2}.\d{6} <= \d{2}.\d{6}$',
				line):
			evac_time.append(float(re.split('<=', line)[-2])) ## The second to last number is the evac time

		else:
			pass

	log.close()

	return evac_time