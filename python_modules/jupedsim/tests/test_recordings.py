# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later
import sqlite3

import pytest

from jupedsim.recording import Recording


def test_can_read_db():
    db_name = "file::memory:?cache=shared"
    db = sqlite3.connect(db_name, uri=True)
    with db:
        db.execute(
            "CREATE TABLE metadata(key TEXT NOT NULL, value TEXT NOT NULL)"
        )
        db.executemany(
            "INSERT INTO metadata VALUES(?, ?)", (("version", 1), ("fps", 25))
        )
        db.execute("CREATE TABLE geometry(wkt TEXT NOT NULL)")
        db.execute(
            "INSERT INTO geometry VALUES(?)",
            ("GEOMETRYCOLLECTION(POLYGON((0 0, 0 1, 1 1, 1 0, 0 0)))",),
        )
        db.execute(
            "CREATE TABLE trajectory_data ("
            "   frame INTEGER NOT NULL,"
            "   id INTEGER NOT NULL,"
            "   pos_x REAL NOT NULL,"
            "   pos_y REAL NOT NULL,"
            "   ori_x REAL NOT NULL,"
            "   ori_y REAL NOT NULL)"
        )
        data = [
            (0, 1, 1.1, 1.1, 0.9, 0.1),
            (0, 2, 1.1, 1.1, 0.9, 0.1),
            (0, 3, 1.1, 1.1, 0.9, 0.1),
            (1, 1, 1.1, 1.1, 0.9, 0.1),
            (1, 2, 1.1, 1.1, 0.9, 0.1),
            (1, 3, 1.1, 1.1, 0.9, 0.1),
            (2, 1, 1.1, 1.1, 0.9, 0.1),
            (2, 2, 1.1, 1.1, 0.9, 0.1),
            (2, 3, 1.1, 1.1, 0.9, 0.1),
        ]
        db.executemany("INSERT INTO trajectory_data VALUES(?,?,?,?,?,?)", data)
    rec = Recording(db_name, uri=True)
    for frame_index in range(0, 3):
        frame = rec.frame(frame_index)
        assert frame.index == frame_index
        for agent, test_data in zip(
            frame.agents, data[frame_index * 3 : frame_index * 3 + 3]
        ):
            assert agent.id == test_data[1]
            assert agent.position == (test_data[2], test_data[3])
            assert agent.orientation == (test_data[4], test_data[5])
    assert rec.geometry() is not None
