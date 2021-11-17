#!/usr/bin/env python3

import sys
import sqlite3


def main():
    conn = sqlite3.connect("events.jps")
    cur = conn.cursor()
    cur.execute(
        """
        CREATE TABLE create_pedestrian_events(
            id INTEGER PRIMARY KEY,
            at_ms INTEGER,
            x REAL,
            y REAL,
            z REAL,
            destinationId INTEGER,
            groupId INTEGER,
            routerId INTEGER,
            patience_ms INTEGER,
            premovement_ms INTEGER,
            riskTolerance INTEGER,
            ellipsisA_V REAL,
            ellipsisAMin REAL,
            ellipsisBMax REAL,
            ellipsisBMin REAL,
            ellipsisStretch INTEGER,
            tau REAL,
            t REAL,
            V0 REAL,
            V0UpStairs REAL,
            V0DownStairs REAL,
            V0EscalatorUp REAL,
            V0EscalatorDown REAL,
            smoothFactorUpStaris REAL,
            smoothFactorDownStaris REAL,
            smoothFactorEscalatorUp REAL,
            smoothFactorEscalatorDown REAL
        )
        """
    )


if __name__ == "__main__":
    sys.exit(main())
