# Copyright © 2012-2024 Forschungszentrum Jülich GmbH
# SPDX-License-Identifier: LGPL-3.0-or-later

"""This code is used in examples on jupedsim.org.

We make no promises about the functions from this file w.r.t. API stability. We
reservere us the right to change the code here w.o. warning. Do not use the
code here. Use it at your own peril. 
"""

import sqlite3

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import pedpy
import plotly.graph_objects as go

DUMMY_SPEED = -1000


def read_sqlite_file(
    trajectory_file: str,
) -> tuple[pedpy.TrajectoryData, pedpy.WalkableArea]:
    """ """
    with sqlite3.connect(trajectory_file) as con:
        data = pd.read_sql_query(
            "select frame, id, pos_x as x, pos_y as y, ori_x as ox, ori_y as oy from trajectory_data",
            con,
        )
        fps = float(
            con.cursor()
            .execute("select value from metadata where key = 'fps'")
            .fetchone()[0]
        )
        walkable_area = (
            con.cursor().execute("select wkt from geometry").fetchone()[0]
        )
        return (
            pedpy.TrajectoryData(data=data, frame_rate=fps),
            pedpy.WalkableArea(walkable_area),
        )


def _speed_to_color(speed, min_speed, max_speed):
    """Map a speed value to a color using a colormap."""
    normalized_speed = (speed - min_speed) / (max_speed - min_speed)
    r, g, b = plt.cm.jet_r(normalized_speed)[:3]
    return f"rgba({r*255:.0f}, {g*255:.0f}, {b*255:.0f}, 0.5)"


def _get_line_color(disk_color):
    r, g, b, _ = [int(float(val)) for val in disk_color[5:-2].split(",")]
    brightness = (r * 299 + g * 587 + b * 114) / 1000
    return "black" if brightness > 127 else "white"


def _create_orientation_line(row, line_length=0.2, color="black"):
    end_x = row["x"] + line_length * row["ox"]
    end_y = row["y"] + line_length * row["oy"]

    orientation_line = go.layout.Shape(
        type="line",
        x0=row["x"],
        y0=row["y"],
        x1=end_x,
        y1=end_y,
        line=dict(color=color, width=3),
    )
    return orientation_line


def _get_geometry_traces(area):
    geometry_traces = []
    x, y = area.exterior.xy
    geometry_traces.append(
        go.Scatter(
            x=np.array(x),
            y=np.array(y),
            mode="lines",
            line={"color": "grey"},
            showlegend=False,
            name="Exterior",
            hoverinfo="name",
        )
    )
    for inner in area.interiors:
        xi, yi = zip(*inner.coords[:])
        geometry_traces.append(
            go.Scatter(
                x=np.array(xi),
                y=np.array(yi),
                mode="lines",
                line={"color": "grey"},
                showlegend=False,
                name="Obstacle",
                hoverinfo="name",
            )
        )
    return geometry_traces


def _get_colormap(frame_data, max_speed):
    """Utilize scatter plots with varying colors for each agent instead of individual shapes.

    This trace is only to incorporate a colorbar in the plot.
    """
    scatter_trace = go.Scatter(
        x=frame_data["x"],
        y=frame_data["y"],
        mode="markers",
        marker=dict(
            size=frame_data["radius"] * 2,
            color=frame_data["speed"],
            colorscale="Jet_r",
            colorbar=dict(title="Speed [m/s]"),
            cmin=0,
            cmax=max_speed,
        ),
        text=frame_data["speed"],
        showlegend=False,
        hoverinfo="none",
    )

    return [scatter_trace]


def _get_shapes_for_frame(frame_data, min_speed, max_speed):
    def create_shape(row):
        hover_trace = go.Scatter(
            x=[row["x"]],
            y=[row["y"]],
            text=[f"ID: {row['id']}, Pos({row['x']:.2f},{row['y']:.2f})"],
            mode="markers",
            marker=dict(size=1, opacity=1),
            hoverinfo="text",
            showlegend=False,
        )
        if row["speed"] == DUMMY_SPEED:
            dummy_trace = go.Scatter(
                x=[row["x"]],
                y=[row["y"]],
                mode="markers",
                marker=dict(size=1, opacity=0),
                hoverinfo="none",
                showlegend=False,
            )
            return (
                go.layout.Shape(
                    type="circle",
                    xref="x",
                    yref="y",
                    x0=row["x"] - row["radius"],
                    y0=row["y"] - row["radius"],
                    x1=row["x"] + row["radius"],
                    y1=row["y"] + row["radius"],
                    line=dict(width=0),
                    fillcolor="rgba(255,255,255,0)",  # Transparent fill
                ),
                dummy_trace,
                _create_orientation_line(row, color="rgba(255,255,255,0)"),
            )
        color = _speed_to_color(row["speed"], min_speed, max_speed)
        return (
            go.layout.Shape(
                type="circle",
                xref="x",
                yref="y",
                x0=row["x"] - row["radius"],
                y0=row["y"] - row["radius"],
                x1=row["x"] + row["radius"],
                y1=row["y"] + row["radius"],
                line_color=color,
                fillcolor=color,
            ),
            hover_trace,
            _create_orientation_line(row, color=_get_line_color(color)),
        )

    results = frame_data.apply(create_shape, axis=1).tolist()
    shapes = [res[0] for res in results]
    hover_traces = [res[1] for res in results]
    arrows = [res[2] for res in results]
    return shapes, hover_traces, arrows


def _create_fig(
    initial_agent_count,
    initial_shapes,
    initial_arrows,
    initial_hover_trace,
    initial_scatter_trace,
    geometry_traces,
    frames,
    steps,
    area_bounds,
    width=800,
    height=800,
    title_note: str = "",
):
    """Creates a Plotly figure with animation capabilities.

    Returns:
        go.Figure: A Plotly figure with animation capabilities.
    """

    minx, miny, maxx, maxy = area_bounds
    title = f"<b>{title_note + '  |  ' if title_note else ''}Number of Agents: {initial_agent_count}</b>"
    fig = go.Figure(
        data=geometry_traces + initial_scatter_trace
        # + hover_traces
        + initial_hover_trace,
        frames=frames,
        layout=go.Layout(
            shapes=initial_shapes + initial_arrows, title=title, title_x=0.5
        ),
    )
    fig.update_layout(
        updatemenus=[_get_animation_controls()],
        sliders=[_get_slider_controls(steps)],
        autosize=False,
        width=width,
        height=height,
        xaxis=dict(range=[minx - 0.5, maxx + 0.5]),
        yaxis=dict(
            scaleanchor="x", scaleratio=1, range=[miny - 0.5, maxy + 0.5]
        ),
    )

    return fig


def _get_animation_controls():
    """Returns the animation control buttons for the figure."""
    return {
        "buttons": [
            {
                "args": [
                    None,
                    {
                        "frame": {"duration": 100, "redraw": True},
                        "fromcurrent": True,
                    },
                ],
                "label": "Play",
                "method": "animate",
            },
        ],
        "direction": "left",
        "pad": {"r": 10, "t": 87},
        "showactive": False,
        "type": "buttons",
        "x": 0.1,
        "xanchor": "right",
        "y": 0,
        "yanchor": "top",
    }


def _get_slider_controls(steps):
    """Returns the slider controls for the figure."""
    return {
        "active": 0,
        "yanchor": "top",
        "xanchor": "left",
        "currentvalue": {
            "font": {"size": 20},
            "prefix": "Frame:",
            "visible": True,
            "xanchor": "right",
        },
        "transition": {"duration": 100, "easing": "cubic-in-out"},
        "pad": {"b": 10, "t": 50},
        "len": 0.9,
        "x": 0.1,
        "y": 0,
        "steps": steps,
    }


def _get_processed_frame_data(data_df, frame_num, max_agents):
    """Process frame data and ensure it matches the maximum agent count."""
    frame_data = data_df[data_df["frame"] == frame_num]
    agent_count = len(frame_data)
    dummy_agent_data = {"x": 0, "y": 0, "radius": 0, "speed": DUMMY_SPEED}
    while len(frame_data) < max_agents:
        dummy_df = pd.DataFrame([dummy_agent_data])
        frame_data = pd.concat([frame_data, dummy_df], ignore_index=True)
    return frame_data, agent_count


def animate(
    data: pedpy.TrajectoryData,
    area: pedpy.WalkableArea,
    *,
    every_nth_frame: int = 50,
    width: int = 800,
    height: int = 800,
    radius: float = 0.2,
    title_note: str = "",
):
    data_df = pedpy.compute_individual_speed(
        traj_data=data,
        frame_step=5,
        speed_calculation=pedpy.SpeedCalculation.BORDER_SINGLE_SIDED,
    )
    data_df = data_df.merge(data.data, on=["id", "frame"], how="left")
    data_df["radius"] = radius
    min_speed = data_df["speed"].min()
    max_speed = data_df["speed"].max()
    max_agents = data_df.groupby("frame").size().max()
    frames = []
    steps = []
    unique_frames = data_df["frame"].unique()
    selected_frames = unique_frames[::every_nth_frame]
    geometry_traces = _get_geometry_traces(area.polygon)
    initial_frame_data = data_df[data_df["frame"] == data_df["frame"].min()]
    initial_agent_count = len(initial_frame_data)
    (
        initial_shapes,
        initial_hover_trace,
        initial_arrows,
    ) = _get_shapes_for_frame(initial_frame_data, min_speed, max_speed)
    color_map_trace = _get_colormap(initial_frame_data, max_speed)
    for frame_num in selected_frames:
        frame_data, agent_count = _get_processed_frame_data(
            data_df, frame_num, max_agents
        )
        shapes, hover_traces, arrows = _get_shapes_for_frame(
            frame_data, min_speed, max_speed
        )
        title = f"<b>{title_note + '  |  ' if title_note else ''}Number of Agents: {agent_count}</b>"
        frame_name = str(int(frame_num))
        frame = go.Frame(
            data=geometry_traces + hover_traces,
            name=frame_name,
            layout=go.Layout(
                shapes=shapes + arrows,
                title=title,
                title_x=0.5,
            ),
        )
        frames.append(frame)

        step = {
            "args": [
                [frame_name],
                {
                    "frame": {"duration": 100, "redraw": True},
                    "mode": "immediate",
                    "transition": {"duration": 500},
                },
            ],
            "label": frame_name,
            "method": "animate",
        }
        steps.append(step)

    return _create_fig(
        initial_agent_count,
        initial_shapes,
        initial_arrows,
        initial_hover_trace,
        color_map_trace,
        geometry_traces,
        frames,
        steps,
        area.bounds,
        width=width,
        height=height,
        title_note=title_note,
    )
