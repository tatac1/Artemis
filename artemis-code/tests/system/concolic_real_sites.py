#!/usr/bin/env python

"""
A Test Suite for running the concolic mode on some real examples.

The examples to use are taken from a CSV file passed as the first argument.
The optional argument "dryrun" just prints the commands that would have been executed.
The optional argument "external-ep-finder" uses an external tool to choose the buttons to analyse.

The CSV file has there columns: The unique name we use to identify the site, the URL which leads to the form, and the
entry-point to be used. The entry-point can either be an XPath expression or the word 'auto' to use the automatic 
entry-points which are hard-coded in ArtForm. If the argument 'external-ep-finder' is used, then this column is ignored
and the external entrypoint-finding tool is used instead. The first line is ignored as a header.

The results from each run are saved in the following folder structure:
./Real-Site-Teating_<date>/<site name>/{<execution tree>, <overview tree>, <constraint log>}

The statistics generated by Artemis are saved to a Google Doc for logging and discussion.
"""

import sys
import os
import unittest
import csv
import time
import datetime
import subprocess
import re
import shutil
import argparse

from harness.artemis import execute_artemis
from harness.artemis import ARTEMIS_EXEC
from harness.artemis import ArtemisCallException
from harness.entrypoint_finder import call_ep_finder

try:
    from harness.test_logging import GDataLogger, current_git_commit_hyperlink
except ImportError:
    print "The 'gdata' module is required to save the results."
    print "See http://code.google.com/p/gdata-python-client/ for downloads."
    sys.exit()



#SPREADSHEET_KEY = "0ApQcHUu6OpaUdDZ5TTR1UlZJYWd1U2ktM0o2YlFoX3c" # Testing
SPREADSHEET_KEY = "0ApQcHUu6OpaUdFZJZEV6LXU2VkZZa1M3QTh6TFAwUWc" # Real Log
WORKSHEET_ID = "od6"

EP_VISUALISATION_SCRIPT = os.path.join(os.environ['ARTEMISDIR'], 'artemis-code', 'scripts', 'entrypoint-identifier.sh')


# The unit test object. The test_* functions will be filled in by main().
class TestSequence(unittest.TestCase):
    pass


def main():
    """Reads the CSV file and runs the tests."""
    print "ArtForm concolic mode test suite"
    
    # Check the arguments
    parser = DefaultHelpParser(description="Runs the test suite of real sites.")
    parser.add_argument('csv_file', help="The CSV file listing the tests to be run. It contains three columns: unique "
                                         "site name, url, entry-point (XPath or 'auto'). The first row is a header.")
    parser.add_argument('--dryrun', action='store_true', dest='dry_run',
                        help="Just prints the commands that would have been executed.")
    parser.add_argument('--external-ep-finder', action='store_true', dest='external_ep_finder',
                        help="Use DIADEM instead of the entry-points listed in the CSV file.")
    args = parser.parse_args()
    dry_run = args.dry_run
    external_ep_finder = args.external_ep_finder
    
    # Read the CSV file
    sites = _read_csv_file(args.csv_file)
    
    # Open the google spreadsheet used for logging
    if not dry_run:
        logger = GDataLogger(SPREADSHEET_KEY, WORKSHEET_ID)
        logger.open_spreadsheet()
    else:
        logger=None
    
    # Create a directory for the test results from this run
    date_string = time.strftime("%Y-%m-%d %H:%M:%S")
    run_dir_name = "Test Suite Run %s" % date_string
    if not dry_run:
        os.mkdir(run_dir_name)
    
    # Check the version of Artemis we are using (by git commit)
    artemis_version_url = current_git_commit_hyperlink()
    
    # Generate a method to test each site and add it to our unit-testable class.
    ep_log = []
    for s in sites:
        test_name = 'test_%s' % re.sub(r'\s+', '', s[0])
        # Check how we will find the EPs.
        if external_ep_finder:
            test = full_test_generator(s[0], s[1], dry_run, logger, artemis_version_url, date_string, run_dir_name, ep_log)
        else:
            test = test_generator(s[0], s[1], s[2], dry_run, logger, artemis_version_url, date_string, run_dir_name)
        
        setattr(TestSequence, test_name, test)
    
    # Run the unit tests
    print "Starting tests..."
    suite = unittest.TestLoader().loadTestsFromTestCase(TestSequence)
    unittest.TextTestRunner(verbosity=2).run(suite)
    
    # Save the EPs which were used.
    if external_ep_finder:
        _save_ep_log(os.path.join(run_dir_name, "ep-log.csv"), ep_log)



def full_test_generator(site_name, site_url, dry_run=False, logger=None, version="", test_date="", test_dir=".", ep_log=None):
    """
    Returns a function which does a 'full' test of the given site.
    This means running the external entry-point finding tool and then for each EP found running the test returned by 
    test_generator() for that site and entry-point.
    The returned function is careful not to allow any exceptions from individual tests to interfere with the others.
    """
    
    def full_test(self):
        os.makedirs(os.path.join(test_dir, site_name))
        
        # Run the entry-point finder.
        try:
            start_time = time.time()
            ep_list = call_ep_finder(site_url, os.path.join(test_dir, site_name))
            end_time = time.time()
            ep_finder_time = str(datetime.timedelta(seconds=(end_time - start_time)))
        except Exception as e:
            # Log this and stop (with the same exception).
            _log_error_message(logger, site_name, site_url, version, test_date, 
                              str(datetime.timedelta(seconds=(time.time() - start_time))),
                              "Exception of type '%s' in test suite while trying to run DIADEM." % type(e).__name__)
            raise
        
        # If we are doing a dry-run then the entry-point finder will not have been called, so use a dummy EP.
        if dry_run:
            ep_list = ["EP-FROM-EXTERNAL-TOOL"]
        
        # If there were no entry-points returned, then log this and stop.
        # This is not considered an error, so no exception is raised.
        if not ep_list:
            _log_error_message(logger, site_name, site_url, version, test_date, ep_finder_time,
                               "DIADEM returned no entry-points.")
            return
        
        # Call the entrypoint visualiser, which is just part of the logging of this test suite. Suppress output.
        #TODO: This crashes too often on real sites, so disabling it until we can work out what is going on.
        #cmd = [EP_VISUALISATION_SCRIPT, site_url, 'buttons.png'] + ep_list
        #try:
        #    subprocess.check_output(cmd, cwd=os.path.join(test_dir, site_name), stderr=subprocess.STDOUT)
        #except CalledProcessError:
        #    pass
        
        # For each EP returned, call test_generator() to get a function to test that EP.
        test_functions = []
        for idx, ep in enumerate(ep_list):
            site_id = "%s_%d" % (site_name, idx+1)
            test_functions.append((site_id, test_generator(site_id, site_url, ep, dry_run, logger, version,
                                                         test_date, test_dir, ep_finder_time)))
            ep_log.append((site_id, site_url, ep))
        
        # Run each of these functions to actually test the different EPs.
        # Keep track of any exceptions (so we can report them at the end) but do not allow them to pass through.
        test_exceptions = []
        print
        for site_id, test_site in test_functions:
            try:
                test_site(None) # The functions returned by test_generator expect to be attached to an object (and 
                                # therefore have the self parameter, but this is never used.
                print "    %s: OK" % site_id
            except Exception as e:
                test_exceptions.append((site_id, type(e).__name__))
                print "    %s: ERROR" % site_id
                print "        ", e
        
        # If there have been any errors, report them and throw an exception to show that this test was not completely
        # successful.
        if test_exceptions:
            error_list = ["%s in %s" % (site_exception, site_id) for site_id, site_exception in test_exceptions]
            error_msg = "Errors occurred: %s" % ", ".join(error_list)
            raise TestSuiteException(error_msg) #TODO: Would be good if we could preserve the original locations as well in this message!
    
    return full_test



def test_generator(site_name, site_url, site_ep, dry_run=False, logger=None, version="", test_date="", test_dir=".", 
                   ep_finder_time=""):
    """Returns a function which will test the given site and entry-point when executed."""
    
    def test(self):
        # Begin with the data we know about
        data = {}
        data['Testing Run'] = test_date
        data['Artemis Version'] = version
        data['Site'] = site_name
        data['URL'] = site_url
        data['Entry Point'] = site_ep
        data['DIADEM Time'] = ep_finder_time
        
        try:
            # Clear /tmp/constraintlog so we can get the constraints from this run only.
            if not dry_run:
                open("/tmp/constraintlog", 'w').close()
            
            # Run and time the test
            start_time = time.time()
            report = execute_artemis(site_name, site_url,
                                     iterations=0,
                                     major_mode='concolic',
                                     concolic_tree_output='final-overview',
                                     verbosity='info,fatal,error',
                                     concolic_button=(None if site_ep.lower() == 'auto' else site_ep),
                                     dryrun=dry_run,
                                     output_parent_dir=test_dir,
                                     ignore_artemis_crash=True)
            end_time = time.time()
            
            if dry_run:
                # Only print the command, exit
                return
            
            data['Running Time'] = str(datetime.timedelta(seconds=(end_time - start_time)))
            data['Exit Code'] = str(report['returncode'])
            
            # Copy the constraint log into the current test directory.
            shutil.copyfile("/tmp/constraintlog", os.path.join(test_dir, site_name, "constraint-log.txt"))
            
            # If the return code indicates a failure, check for a core dump and create a backtrace if one exists.
            # We also delete the core dumps to save space!
            core_file = os.path.join(test_dir, site_name, 'core')
            if report['returncode'] != 0 and os.path.isfile(core_file):
                try:
                    bt_cmd = "gdb -q -n -ex bt -batch %s core > backtrace.txt 2>&1" % ARTEMIS_EXEC
                    subprocess.call(bt_cmd, shell=True, cwd=os.path.join(test_dir, site_name));
                    os.remove(core_file)
                except OSError:
                    pass # Ignore any errors in this part.
            
            if logger is not None:
                # Add all concolic statistics from Artemis to the logged data.
                for key in report.keys():
                    key_lc = key.lower()
                    if key_lc.startswith("concolic::") and not key_lc.startswith("concolic::solver::constraint."):
                        col_header = key[10:].replace("::", "::\n")
                        data[col_header] = str(report[key])
                    elif key_lc.startswith("ajax::"):
                        col_header = key[6:].replace("::", "::\n")
                        data[col_header] = str(report[key])
                
                # Append the log data to the google doc.
                logger.log_data(data)
            
        except Exception as e:
            # Catch any exceptions which ocurred during the test suite itself.
            # Try to log a message that this has happened (but ignore any new errors this may cause).
            try:
                if logger is not None:
                    data['Analysis'] = "Exception of type '%s' in the test suite." % type(e).__name__
                    logger.log_data(data)
            except Exception:
                pass
            
            # Re-raise the same exception, as if we had never caught it in the first place.
            #TODO: Actually the traceback is lost. Maybe we could use sys.exc_info() to preserve this?
            raise e
        
        # Fail the test if the return code was non-zero.
        if 'returncode' in report and report['returncode'] != 0:
            raise ArtemisCallException("Exception thrown by call to artemis (returned %d)." % report['returncode'])
    
    return test



def _log_error_message(logger, site_id, site_url, version, test_date, ep_finder_time, message):
    """
    Logs an error message to mark when ArtForm could not be run.
    Silently ignores exceptions during logging, as this is onyl an error-recovery function.
    """
    try:
        if logger is not None:
            data = {}
            data['Testing Run'] = test_date
            data['Artemis Version'] = version
            data['Site'] = site_id
            data['URL'] = site_url
            data['DIADEM Time'] = ep_finder_time
            data['Analysis'] = message
            logger.log_data(data)
    except Exception:
        pass




def _read_csv_file(filename):
    """
    Reads a CSV file into a list of triples.
    If the CSV file is formatted correctly it will return a list of name/URL/EP triples
    """
    sites = []
    try:
        with open(filename, 'rb') as csvfile:
            reader = csv.reader(csvfile, skipinitialspace=True)
            reader.next() # Ignore header row
            for row in reader:
                assert(len(row) == 3) # We expect three arguments.
                sites.append(row)
        return sites
    except IOError:
        print "Could not open file", filename
        exit(1)
    except AssertionError:
        print "Encountered a row with the wrong length, aborting."
        exit(1)


def _save_ep_log(filename, ep_log):
    """Writes a list of (site name, url, xpath) triples to a csv file as a log of the tests which were run."""
    
    with open(filename, 'wb') as logfile:
        writer = csv.writer(logfile)
        writer.writerow(("Site", "URL", "Entry Point"))
        writer.writerows(ep_log)



# An argument parser which prints the help message after an error.
# http://stackoverflow.com/a/3637103/1044484
class DefaultHelpParser(argparse.ArgumentParser):
    def error(self, message):
        sys.stderr.write('Error: %s\n' % message)
        self.print_help()
        sys.exit(2)


class TestSuiteException(Exception):
    pass


if __name__ == "__main__":
    main()