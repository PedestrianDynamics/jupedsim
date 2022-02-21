import xml.etree.cElementTree as ET
from pathlib import Path


def read_starting_times(events_file: Path):
    tree = ET.parse(events_file)
    root = tree.getroot()
    starting_times = {}
    for event in root.iter("event"):
        if event.attrib["state"] == "open" or event.attrib["state"] == "reset":
            starting_times.setdefault(int(event.attrib["id"]), []).append(
                float(event.attrib["time"])
            )

    if not starting_times:
        raise RuntimeError("Could not read starting times from events file")

    return starting_times
