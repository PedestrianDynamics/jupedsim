#! /usr/bin/env python3
import argparse
import io
import logging
import logging as log
import sys
import unittest


class IncorrectTrajectory(Exception):
    def __init__(self, message):
        self.message = message


class Mock:
    pass


class TestTrajectoryClass(unittest.TestCase):
    def test_exception_when_missing_framerate(self):
        f = io.StringIO(
            "\n".join(
                [
                    "#geometry: geo_stairs_1.0m.xml",
                    "#ID	FR	X	Y	Z	A	B	ANGLE	COLOR",
                    "1	0	14.90	6.60	0.00	0.20	0.20	-165.96	0",
                ]
            )
        )
        path = "no path"
        with self.assertRaises(IncorrectTrajectory) as ex:
            Trajectory.parse_from_stream(f, path)
        assert (
            ex.exception.message
            == f"file named '{path}' is missing a frame rate in the Header"
        )

    def test_exception_when_missing_geometry(self):
        f = io.StringIO(
            "\n".join(
                [
                    "#framerate: 64.00",
                    "#ID	FR	X	Y	Z	A	B	ANGLE	COLOR",
                    "1	0	14.90	6.60	0.00	0.20	0.20	-165.96	0",
                ]
            )
        )
        path = "no path"
        with self.assertRaises(IncorrectTrajectory) as ex:
            Trajectory.parse_from_stream(f, path)
        assert (
            ex.exception.message
            == f"file named '{path}' is missing a Geometry in the Header"
        )

    def test_correct_framerate_parsed(self):
        f = io.StringIO(
            "\n".join(
                [
                    "#framerate: 64.00",
                    "#geometry: geo_stairs_1.0m.xml",
                    "#ID	FR	X	Y	Z	A	B	ANGLE	COLOR",
                    "1	0	14.90	6.60	0.00	0.20	0.20	-165.96	0",
                ]
            )
        )
        path = "no path"
        trajec_data = Trajectory.parse_from_stream(f, path)
        self.assertEqual(trajec_data[0], "64.00")

    def test_correct_geometry_parsed(self):
        f = io.StringIO(
            "\n".join(
                [
                    "#framerate: 64.00",
                    "#geometry: geo_stairs_1.0m.xml",
                    "#ID	FR	X	Y	Z	A	B	ANGLE	COLOR",
                    "1	0	14.90	6.60	0.00	0.20	0.20	-165.96	0",
                ]
            )
        )
        path = "no path"
        trajec_data = Trajectory.parse_from_stream(f, path)
        self.assertEqual(trajec_data[1], "geo_stairs_1.0m.xml")

    def test_exception_if_file_not_found(self):
        path = r"this is no filename # % & { }"
        with self.assertRaises(IncorrectTrajectory):
            Trajectory(path)

    def test_exception_when_missing_frames(self):
        f = io.StringIO(
            "\n".join(
                [
                    "#framerate: 64.00",
                    "#geometry: geo_stairs_1.0m.xml",
                    "#ID	FR	X	Y	Z	A	B	ANGLE	COLOR",
                    "1	0	14.90	6.60	0.00	0.20	0.20	-165.96	0",
                    "1	2	14.89	6.60	0.00	0.20	0.20	-159.19	104	",
                ]
            )
        )
        path = "no path"
        with self.assertRaises(IncorrectTrajectory) as ex:
            Trajectory.parse_from_stream(f, path)
        assert (
            ex.exception.message
            == f"file {path} has missing frames ~ missed frame 1"
        )

    def test_exception_when_line_has_no_frame(self):
        wrong_line1 = "this line has no frames and no tabs"
        wrong_line2 = "this line has no frames but   tabs"
        f = io.StringIO(
            "\n".join(
                [
                    "#framerate: 64.00\n",
                    "#geometry: geo_stairs_1.0m.xml\n",
                    "#ID	FR	X	Y	Z	A	B	ANGLE	COLOR\n",
                    wrong_line1,
                ]
            )
        )
        f2 = io.StringIO(
            "\n".join(
                [
                    "#framerate: 64.00\n",
                    "#geometry: geo_stairs_1.0m.xml\n",
                    "#ID	FR	X	Y	Z	A	B	ANGLE	COLOR\n",
                    wrong_line2,
                ]
            )
        )
        path = "no path"
        with self.assertRaises(IncorrectTrajectory) as ex:
            Trajectory.parse_from_stream(f, path)
        assert (
            ex.exception.message
            == f"file named '{path}' has wrong formatted Line: {wrong_line1}"
        )
        with self.assertRaises(IncorrectTrajectory) as ex:
            Trajectory.parse_from_stream(f2, path)
        assert (
            ex.exception.message
            == f"file named '{path}' has wrong formatted Line: {wrong_line2}"
        )


class TestCheckInput(unittest.TestCase):
    def test_exception_when_framerates_dont_match(self):
        paths = ["no path", "still no path"]
        frame_rates = ["64.00", "32.00"]
        geometries = ["geo1.xml", "geo1.xml"]
        with self.assertRaises(IncorrectTrajectory) as ex:
            check_header((paths, frame_rates, geometries))
        assert (
            ex.exception.message
            == f"the frame rate in file {paths[1]} is not matching"
        )

    def test_exception_when_geometries_dont_match(self):
        paths = ["no path", "still no path"]
        frame_rates = ["64.00", "64.00"]
        geometries = ["geo1.xml", "geo2.xml"]
        with self.assertRaises(IncorrectTrajectory) as ex:
            check_header((paths, frame_rates, geometries))
        assert (
            ex.exception.message
            == f"the Geometry path in file {paths[1]} is not matching"
        )

    def test_exception_multiple_startvalues(self):
        with self.assertRaises(IncorrectTrajectory) as ex:
            end_frames = [4777, 8857, 12470, 15761, 18817, 21684, 24393, 26178]
            start_frames = [0, 0, 8858, 12471, 15762, 18818, 21685, 24394]
            check_data((start_frames, end_frames))
        assert ex.exception.message == f"multiple files start with the frame 0"

    def test_exception_frames_miss_between_files(self):
        with self.assertRaises(IncorrectTrajectory) as ex:
            start_frames = [0, 4500, 9000, 12500]
            end_frames = [5000, 8999, 12499, 15000]
            check_data((start_frames, end_frames))
        assert (
            ex.exception.message
            == f"the starting frame 4500 is not matching the other files"
        )

    def test_exception_frames_overlap(self):
        with self.assertRaises(IncorrectTrajectory) as ex:
            start_frames = [0, 4500, 9000, 12500]
            end_frames = [4499, 9500, 12499, 15000]
            check_data((start_frames, end_frames))
        assert (
            ex.exception.message
            == f"the starting frame 9000 is not matching the other files"
        )

    def test_read_header(self):
        trajec1 = Mock()
        trajec1.path, trajec1.framerate, trajec1.geometry = (
            "no path",
            "64.00",
            "geo1.xml",
        )
        trajec2 = Mock()
        trajec2.path, trajec2.framerate, trajec2.geometry = (
            "still no path",
            "32.00",
            "geo1.xml",
        )
        trajecs = [trajec1, trajec2]
        actual = read_header(trajecs)
        act_paths = actual[0]
        act_frame_rates = actual[1]
        act_geometries = actual[2]
        exp_paths = ["no path", "still no path"]
        exp_frame_rates = ["64.00", "32.00"]
        exp_geometries = ["geo1.xml", "geo1.xml"]
        self.assertEqual(act_paths, exp_paths)
        self.assertEqual(act_frame_rates, exp_frame_rates)
        self.assertEqual(act_geometries, exp_geometries)

    def test_read_data(self):
        trajec1, trajec2, trajec3, trajec4 = Mock(), Mock(), Mock(), Mock()
        trajec1.start_frame, trajec1.end_frame = 0, 4999
        trajec2.start_frame, trajec2.end_frame = 5000, 9999
        trajec3.start_frame, trajec3.end_frame = 10000, 14999
        trajec4.start_frame, trajec4.end_frame = 15000, 20000
        trajecs = [trajec1, trajec2, trajec3, trajec4]
        acutal = read_data(trajecs)
        act_start_frames = acutal[0]
        act_end_frames = acutal[1]
        exp_start_frames = [0, 5000, 10000, 15000]
        exp_end_frames = [4999, 9999, 14999, 20000]
        self.assertEqual(exp_start_frames, act_start_frames)
        self.assertEqual(exp_end_frames, act_end_frames)


class TestFrameFromLine(unittest.TestCase):
    def test_for_correct_frame(self):
        self.assertEqual(
            frame_from_line("1	0	14.90	6.60	0.00	0.20	0.20	-165.96	0\n"), 0
        )

    def test_for_incorrect_format(self):
        with self.assertRaises(IndexError):
            frame_from_line("this is not an actual line")

    def test_for_incorrect_type(self):
        with self.assertRaises(ValueError):
            frame_from_line(
                "1	no number	14.90	6.60	0.00	0.20	0.20	-165.96	0\n"
            )


class TestOutput(unittest.TestCase):
    def test_framerate_found(self):
        head = get_header(64, "geometry.xml")
        split = head.split("framerate: ")
        temp = split[1]
        framerate = int(temp[0:3].strip("\n"))
        self.assertEqual(framerate, 64)

    def test_geometry_found(self):
        head = get_header(64, "geometry.xml")
        lines = head.split("\n")
        geometry = "not found"
        for line in lines:
            if line.startswith("#geometry"):
                split_line = line.split(": ")
                geometry = split_line[1].strip("\n")
        self.assertEqual(geometry, "geometry.xml")

    def test_string_from_trajecs(self):
        lines = [
            "#framerate: 64.00",
            "#geometry: geo_stairs_1.0m.xml",
            "#ID	FR	X	Y	Z	A	B	ANGLE	COLOR",
            "",
            "1	0	14.90	6.60	0.00	0.20	0.20	-165.96	0",
            "18	0	16.10	3.40	0.00	0.20	0.20	-178.53	0",
        ]
        f = io.StringIO("\n".join(lines))
        actual = entries_from_stream(f)
        expected = "\n".join(lines[4:])
        self.assertEqual(actual, expected)


class Trajectory:
    def __init__(self, path):
        """initialises frame rate and geometry, the starting Frame and the last Frame"""
        self.path = path
        try:
            with open(self.path, "r") as trajec:
                (
                    self.framerate,
                    self.geometry,
                    self.start_frame,
                    self.end_frame,
                ) = Trajectory.parse_from_stream(trajec, self.path)

        except FileNotFoundError as error:
            raise IncorrectTrajectory(
                f"the file '{error.filename}' can not be found"
            )
        except (IndexError, ValueError):
            # if this Error occurs no Frame could be found in a line
            raise IncorrectTrajectory(
                f"the file '{self.path}' is no correct Trajectory-file"
            )
        except OSError:
            raise IncorrectTrajectory(
                f'the path "{path}" is not allowed with our operation system'
            )

    def __lt__(self, other):
        return self.start_frame < other.start_frame

    @staticmethod
    def parse_from_stream(trajec, path):
        """determines frame rate, geometry, start frame and end frame
        throws an exception if a frame is missing in the file"""
        framerate, geometry, start_frame, end_frame = None, None, None, None
        frame_found = False
        geometry_found = False
        for line in trajec:
            if line == "\n" or line.startswith("#"):
                if line.startswith("#framerate"):
                    split_line = line.split(": ")
                    value = split_line[1]
                    value = value.strip("\n")
                    framerate = value
                    frame_found = True
                if line.startswith("#geometry"):
                    split_line = line.split(": ")
                    value = split_line[1]
                    value = value.strip("\n")
                    geometry = value
                    geometry_found = True
                continue
            else:
                try:
                    value = frame_from_line(line)
                    start_frame = value
                    break
                except (ValueError, IndexError):
                    raise IncorrectTrajectory(
                        f"file named '{path}' has wrong formatted Line: {line}"
                    )

        if not frame_found:
            raise IncorrectTrajectory(
                f"file named '{path}' is missing a frame rate in the Header"
            )
        if not geometry_found:
            raise IncorrectTrajectory(
                f"file named '{path}' is missing a Geometry in the Header"
            )

        # this will continue iteration where we left off the last time
        # All headers and empty lines are skipped only frame lines remain
        last_frame = start_frame
        try:
            for line in trajec:
                current_frame = frame_from_line(line)
                if current_frame != last_frame and current_frame != (
                    last_frame + 1
                ):
                    raise IncorrectTrajectory(
                        f"file {path} has missing frames ~ missed frame {last_frame + 1}"
                    )
                last_frame = current_frame
            # if the file is completely iterated through all Frames are included
        except (ValueError, IndexError):
            raise IncorrectTrajectory(
                f"file named '{path}' has wrong formatted Line: {line}"
            )
        value = last_frame
        end_frame = value
        return framerate, geometry, start_frame, end_frame


def frame_from_line(line):
    frame = line.split("\t")
    return int(frame[1])


def read_header(trajecs):
    """reads all frame rates and geometries
    returns a tuple containing (paths, frame_rates, geometries)"""
    frame_rates, geometries, paths = [], [], []

    for trajec in trajecs:
        paths.append(trajec.path)
        frame_rates.append(trajec.framerate)
        geometries.append(trajec.geometry)
    return paths, frame_rates, geometries


def check_header(data):
    """checks if frame rates and geometries match
    check header expects a tuple formatted like (paths, frame_rates, geometries)
    """
    paths, frame_rates, geometries = data
    values = zip(paths, frame_rates, geometries)
    # Since all geometry filenames and fps values have to be the same we just pick the first one
    _, expected_frame_rate, expected_geometry_file = next(values)
    for path, frame_rate, geometry_file in values:
        if frame_rate != expected_frame_rate:
            raise IncorrectTrajectory(
                f"the frame rate in file {path} is not matching"
            )
        if geometry_file != expected_geometry_file:
            raise IncorrectTrajectory(
                f"the Geometry path in file {path} is not matching"
            )
    return True


def read_data(trajecs):
    """collects all starting- and end frames then checks if they match"""
    end_frames = []
    start_frames = []
    for trajec in trajecs:
        start_frames.append(trajec.start_frame)
        end_frames.append(trajec.end_frame)
    return start_frames, end_frames


def check_data(starting_and_ending_frames):
    """checks if starting Frames and end Frames are matching one Trajectory
    starting_and_ending_Frames is expected to be a tuple formatted like (start_Frames, end_Frames)
    """

    start_frames = starting_and_ending_frames[0]
    end_frames = starting_and_ending_frames[1]
    for start_frame in start_frames:
        # a starting frame is considered valid if it is the first frame of the given trajectories
        # a starting frame is considered valid if it comes after an end frame
        if (
            start_frame != min(start_frames)
            and start_frame - 1 not in end_frames
        ):
            raise IncorrectTrajectory(
                f"the starting frame {start_frame} is not matching the other files"
            )
        # a starting frame is invalid if it appears more than once
        if start_frames.count(start_frame) > 1:
            raise IncorrectTrajectory(
                f"multiple files start with the frame {start_frame}"
            )
    return True


def merge_trajectories(trajecs, output, verbose):
    """will merge all data into the output-file
    "trajecs" is a list of all Trajectory files in Class format"""

    # Trajectories are sorted after starting Frame
    trajecs.sort()
    debug_length = len(trajecs)

    for i, trajectory in enumerate(trajecs):
        if verbose:
            log.debug(f"writing file {i + 1}/{debug_length}")
        append_to_output(trajectory.path, output)

    log.info(f'new File "{output}" was created.')


def append_to_output(trajec_path, output):
    """opens the file, gets the important data, then appends it to the outputfile"""
    with open(trajec_path, "r") as input_file:
        text = entries_from_stream(input_file)
    write_text(text, output, "a")


def entries_from_stream(stream):
    """returns all lines from a trajectory that need to be included in the merged file"""
    lines = []
    for line in stream:
        if line != "\n" and not line.startswith("#"):
            lines.append(line)
    return "".join(lines)


def get_header(framerate, geometry):
    return (
        f"#merged with MergeTool\n"
        f"#framerate: {framerate}\n"
        f"#geometry: {geometry}\n\n"
    )


def write_text(text, output, mode):
    with open(output, mode) as output_file:
        output_file.write(text)


def configuration():
    """sets the Logger and Argumentparser up"""
    log.basicConfig(format="%(levelname)s:%(message)s", level=logging.DEBUG)
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "file",
        type=str,
        help="Merge the given Trajectory files together",
        nargs="+",
    )
    parser.add_argument(
        "-o",
        type=str,
        required=False,
        default="merged_Trajectories.txt",
        help="define output file, default path is merged_Trajectories.txt",
        dest="output_path",
    )
    parser.add_argument(
        "-v",
        action="store_true",
        help="activates verbose mode and shows how many files remain",
        dest="verbose",
    )
    parser.add_argument(
        "--self-test",
        action="store_true",
        help="Will run self tests, print the results and exit.",
    )
    args = parser.parse_args()
    return args


def main():
    args = configuration()

    if args.self_test:
        unit_test_args = [sys.argv[0]]
        unittest.main(argv=unit_test_args)
        sys.exit(0)

    try:
        if args.verbose:
            log.debug("it will be checked if the Trajectories match")

        # creates a list with the file in Class form
        trajecs = []
        for file in args.file:
            trajecs.append(Trajectory(file))

        check_header(read_header(trajecs))
        check_data(read_data(trajecs))
        if args.verbose:
            log.debug("Trajectories match")
    except IncorrectTrajectory as error:
        log.error(error)
        log.info("Use -h for more info on how to use the Script")
        sys.exit(1)
    write_text(
        get_header(trajecs[0].framerate, trajecs[0].geometry),
        args.output_path,
        "w",
    )
    merge_trajectories(trajecs, args.output_path, args.verbose)


if __name__ == "__main__":
    main()
