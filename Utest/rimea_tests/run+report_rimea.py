import runtest_execute
import reportscript

if __name__ == '__main__':
#    runtest_execute.test()
#    reportscript.test()
    runtest_execute.runtest_execute(delete_logs=True)
    reportscript.generate_info_report()
    print('\nreportscript successfully created\n')
