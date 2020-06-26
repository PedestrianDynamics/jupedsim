#!/usr/bin/env python3

import difflib
import filecmp
import os
import logging

from utils import SUCCESS, FAILURE

def check_diff_to_reference_data():

    reference_folder = "Output_expected"
    new_folder = "Output_new"

    expected_files = []

    #create a list of all files that should have been generated
    for root, dirs, files in os.walk(reference_folder):

        #consider .dat files only
        files = [ f for f in files if f.endswith(".dat") ]

        for name in files:
            tmp_path = os.path.join(root, name)
            #remove parent directory
            expected_files.append(os.path.relpath(tmp_path, reference_folder))

    match, mismatch, errors = filecmp.cmpfiles(reference_folder, new_folder, expected_files, shallow=False)

    # print diff for mismatching files
    for file in mismatch:

        expected_file_name = os.path.join(reference_folder, file)
        new_file_name = os.path.join(new_folder, file)

        with open(expected_file_name) as expected_file:
            expected_file_text = expected_file.readlines()
        with open(new_file_name) as new_file:
            new_file_text = new_file.readlines()

        diff_text = ''.join(difflib.context_diff(expected_file_text, new_file_text, fromfile=expected_file_name, tofile=new_file_name))
        logging.critical("jpsreport did not output results correctly. Found diff in: %s " % (diff_text))

    # print name of files that are missing
    for file in errors:
        new_file_name = os.path.join(new_folder, file)
        logging.critical("jpsreport did not output results correctly. File was not created: %s" % (new_file_name))

    if not (mismatch or errors):
        logging.info("OK. Output files of jpsreport have not changed.")
    else:
        exit(FAILURE)
