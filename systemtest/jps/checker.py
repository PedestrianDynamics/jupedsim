"""Functions to check jpscore log files for specific output."""


def output_contains_line_exact(text, line):
    """
    Search for a specific line in the output.

    @param text to search in. This is the output of jpscore usually.
    @param line to search for.
    """
    for line in text.splitlines():
        if line == line:
            return True
    return False
