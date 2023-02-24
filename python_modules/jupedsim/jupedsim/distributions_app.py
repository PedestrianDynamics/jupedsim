import distributions
import matplotlib.pyplot as plt
import streamlit as st


def show_points(
    s_polygon,
    samples,
    radius,
    circle_segment_radii=None,
    center_point=None,
    obstacles=None,
):
    if obstacles is None:
        obstacles = []
    box = distributions.__get_bounding_box(s_polygon)
    exterior = list(s_polygon.exterior.coords)
    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1)
    for elem in samples:
        ax.add_patch(plt.Circle(radius=radius / 2, xy=elem, fill=False))
        ax.add_patch(plt.Circle(radius=0.1, xy=elem, color="r"))

    if circle_segment_radii is not None:
        for circle_segment in circle_segment_radii:
            ax.add_patch(
                plt.Circle(
                    radius=circle_segment[0], xy=center_point, fill=False
                )
            )
            ax.add_patch(
                plt.Circle(
                    radius=circle_segment[1], xy=center_point, fill=False
                )
            )

    n = len(exterior)
    i = 0
    while True:
        following = (i + 1) % n
        x_value = [exterior[i][0], exterior[following][0]]
        y_value = [exterior[i][1], exterior[following][1]]
        plt.plot(x_value, y_value, color="blue")

        i += 1
        if following == 0:
            break

    for obstacle in obstacles:
        n = len(obstacle)
        i = 0
        while True:
            following = (i + 1) % n
            x_value = [obstacle[i][0], obstacle[following][0]]
            y_value = [obstacle[i][1], obstacle[following][1]]
            plt.plot(x_value, y_value, color="black")

            i += 1
            if following == 0:
                break

    plt.xlim(box[0][0], box[1][0])
    plt.ylim(box[0][1], box[1][1])
    plt.axis("equal")
    st.pyplot(fig)


def main():
    default_polygon = [(0.0, 0.0), (10.0, 0.0), (10.0, 10.0), (0.0, 10.0)]
    agents = density = 0

    with st.sidebar:
        st.text("General settings")
        distance_to_agents = st.slider(
            "Distance between the center points of the agents",
            0.1,
            2.0,
            value=0.3,
        )
        distance_to_polygon = st.slider(
            "Distance between agents and walls", 0.1, 2.0, value=0.3
        )
        st.text("Polygon settings")
        corners = st.number_input(
            "Number of edges in polygon", 3, value=len(default_polygon)
        )
        col1, col2 = st.columns(2)
        x_values, y_values = [], []
        with col1:
            for i in range(corners):
                x_values.append(
                    st.number_input(
                        f"X{i + 1}",
                        value=default_polygon[i][0]
                        if i < len(default_polygon)
                        else 0.0,
                        step=1.0,
                    )
                )
        with col2:
            for i in range(corners):
                y_values.append(
                    st.number_input(
                        f"Y{i + 1}",
                        value=default_polygon[i][1]
                        if i < len(default_polygon)
                        else 0.0,
                        step=1.0,
                    )
                )
        seed = st.number_input(
            "Set a seed for random number generation.\n"
            "Agent Distribution with the same settings will lead to the same output.\n"
            "If the seed is set to zero the distribution will be random each time",
            0,
        )
        if seed == 0:
            seed = None
        st.text(
            "Hole settings:\n"
            "Holes define Spaces where no Agents must be placed."
        )
        obstacle_count = st.number_input("Number of Holes", 0, value=0)
        obstacle_corners = []
        obstacle_values = []
        for i in range(obstacle_count):
            obstacle_corners.append(
                st.number_input(f"Number of edges in hole {i + 1}", 3, value=4)
            )
        col3, col4 = st.columns(2)
        with col3:
            for i in range(obstacle_count):
                obstacle_values.append([])
                st.text(f"Settings for")
                # the rest of the text can be found in column 4
                for j in range(obstacle_corners[i]):
                    obstacle_values[i].append(
                        [
                            st.number_input(
                                f"X{j + 1} in hole {i + 1}",
                                value=0.0,
                                step=1.0,
                            )
                        ]
                    )
        with col4:
            for i in range(obstacle_count):
                st.text(f"hole {i + 1}")
                # this text is placed so the text "Settings for hole <i+1> aligns through columns 3 & 4
                for j in range(obstacle_corners[i]):
                    obstacle_values[i][j].append(
                        st.number_input(
                            f"Y{j + 1} in hole {i + 1}", value=0.0, step=1.0
                        )
                    )

    obstacles = []
    for i in range(obstacle_count):
        obstacles.append([])
        for j in range(obstacle_corners[i]):
            obstacles[i].append(
                (obstacle_values[i][j][0], obstacle_values[i][j][1])
            )
    polygon = []
    for x, y in zip(x_values, y_values):
        polygon.append((x, y))
    s_polygon = distributions.shply.Polygon(polygon, obstacles)
    area = s_polygon.area
    distribution_type = st.radio(
        "How to distribute agents?",
        (
            "place by number/density",
            "place in circles",
            "place till full",
            "place by percentage",
        ),
    )

    if distribution_type == "place in circles":
        st.text(
            "Instructions: "
            "Agents will be distributed inside both the polygon and circle segments.\n"
            "Agents do not keep distance to circle segment borders like polygons.\n"
            "You need to select a center point for all circle segments.\n"
            "You also need to select the range of each circle segment.\n"
            "Circle segments musst not overlap or have negativ values.\n"
            "General settings can be made in the left sidebar."
        )
        style = st.radio(
            f"How to choose number of agents distributed inside circle segments?",
            ("density", "number"),
        )
        circle_count = st.number_input("number of circles", 1)
        col1, col2 = st.columns(2)
        min_values, max_values = [], []
        densities, agents = [], []
        default_center = (5.0, 5.0)
        with col1:
            mid_x = st.number_input(
                "Center X", value=default_center[0], step=1.0
            )
            for i in range(circle_count):
                min_values.append(
                    st.number_input(
                        f"inner radius for Circle segment {i + 1}", step=1.0
                    )
                )
        with col2:
            mid_y = st.number_input(
                "Center Y", value=default_center[1], step=1.0
            )
            for i in range(circle_count):
                max_values.append(
                    st.number_input(
                        f"outer radius for Circle segment {i + 1}", step=1.0
                    )
                )

        center_point = (mid_x, mid_y)
        circle_segment_radii = []
        for i in range(circle_count):
            circle_segment_radii.append((min_values[i], max_values[i]))

        for i in range(circle_count):
            if style == "density":
                densities.append(
                    st.slider(
                        f"Agents / m² in Circle segment {i + 1}",
                        0.1,
                        round(
                            (
                                69.2636
                                * (distance_to_agents ** (-1.89412))
                                / 100
                            ),
                            2,
                        ),
                        key=i,
                    )
                )
                # the function for the maximum selectable density is an estimate of the real maximum density possible
                # the estimation was done by Power-Regression
            elif style == "number":
                temp_min, temp_max = (
                    circle_segment_radii[i][0],
                    circle_segment_radii[i][1],
                )
                area_small = distributions.__intersecting_area_polygon_circle(
                    center_point, temp_min, s_polygon
                )
                area_big = distributions.__intersecting_area_polygon_circle(
                    center_point, temp_max, s_polygon
                )
                area_segment = area_big - area_small
                agents.append(
                    st.slider(
                        f"Agents in Circle segment {i + 1}",
                        1,
                        round(area_segment * 8),
                        key=i,
                    )
                )

        button_clicked = st.button("distribute agents")
        if button_clicked:
            samples = []
            if style == "density":
                samples = distributions.distribute_in_circles_by_density(
                    polygon=s_polygon,
                    distance_to_agents=distance_to_agents,
                    distance_to_polygon=distance_to_polygon,
                    center_point=center_point,
                    circle_segment_radii=circle_segment_radii,
                    densities=densities,
                    seed=seed,
                )
            elif style == "number":
                samples = distributions.distribute_in_circles_by_number(
                    polygon=s_polygon,
                    distance_to_agents=distance_to_agents,
                    distance_to_polygon=distance_to_polygon,
                    center_point=center_point,
                    circle_segment_radii=circle_segment_radii,
                    numbers_of_agents=agents,
                    seed=seed,
                )
            st.text(f"Below should be a plot containing {len(samples)} agents")
            show_points(
                s_polygon,
                samples,
                distance_to_agents,
                circle_segment_radii,
                center_point,
                obstacles,
            )
        else:
            show_points(
                s_polygon,
                [],
                distance_to_agents,
                circle_segment_radii,
                center_point,
                obstacles,
            )

    if distribution_type == "place by number/density":
        st.text(
            "Instructions: "
            "a certain amount of agents will be distributed inside the polygon.\n"
            "You can enter this amount either by a specific number or a density.\n"
            "General settings can be made in the left sidebar."
        )
        style = st.radio(
            "How to choose number of agents?", ("density", "number")
        )
        if style == "density":
            density = st.slider(
                "Agents / m²",
                0.1,
                round((69.2636 * (distance_to_agents ** (-1.89412)) / 100), 2),
            )
            # the function for the maximum selectable density is an estimate of the real maximum density possible
            # the estimation was done by Power-Regression
        elif style == "number":
            agents = st.slider("Agents", 1, round(area * 8))

        button_clicked = st.button("distribute agents")

        if button_clicked:
            samples = []
            if style == "density":
                samples = distributions.distribute_by_density(
                    polygon=s_polygon,
                    density=density,
                    distance_to_agents=distance_to_agents,
                    distance_to_polygon=distance_to_polygon,
                    seed=seed,
                )
            elif style == "number":
                samples = distributions.distribute_by_number(
                    polygon=s_polygon,
                    number_of_agents=agents,
                    distance_to_agents=distance_to_agents,
                    distance_to_polygon=distance_to_polygon,
                    seed=seed,
                )
            st.text(f"Below should be a plot containing {len(samples)} agents")
            show_points(
                s_polygon=s_polygon,
                samples=samples,
                radius=distance_to_agents,
                obstacles=obstacles,
            )
        else:
            show_points(
                s_polygon=s_polygon,
                samples=[],
                radius=distance_to_agents,
                obstacles=obstacles,
            )

    if distribution_type == "place till full":
        st.text(
            "Instructions: "
            "agents will be distributed all over the polygon.\n"
            "You can enter how many tries there should be to "
            "find another place to put an agent before a polygon is considered filled.\n"
            "General settings can be made in the left sidebar"
        )
        max_iterations = st.number_input(
            "maximum tries to find the first point", 1, value=10_000, step=100
        )
        k = st.number_input(
            "maximum tries to find a point around a reference point",
            1,
            value=30,
            step=1,
        )
        button_clicked = st.button("distribute agents")

        if button_clicked:
            samples = distributions.distribute_till_full(
                polygon=s_polygon,
                distance_to_agents=distance_to_agents,
                distance_to_polygon=distance_to_polygon,
                seed=seed,
                max_iterations=max_iterations,
                k=k,
            )
            st.text(f"Below should be a plot containing {len(samples)} agents")
            show_points(
                s_polygon=s_polygon,
                samples=samples,
                radius=distance_to_agents,
                obstacles=obstacles,
            )
        else:
            show_points(
                s_polygon=s_polygon,
                samples=[],
                radius=distance_to_agents,
                obstacles=obstacles,
            )

    if distribution_type == "place by percentage":
        st.text(
            "Instructions: "
            "distributes agents inside the polygon.\n"
            "The polygon will be filled to a certain percentage.\n"
            "You can also enter how many tries there should be to "
            "find another place to put an agent before a polygon is considered filled.\n"
            "General settings can be made in the left sidebar"
        )
        percent = st.slider("Percent filled: ", 1, 100)
        max_iterations = st.number_input(
            "maximum tries to find the first point", 1, value=10_000, step=100
        )
        k = st.number_input(
            "maximum tries to find a point around a reference point",
            1,
            value=30,
            step=1,
        )
        button_clicked = st.button("distribute agents")

        if button_clicked:
            samples = distributions.distribute_by_percentage(
                polygon=s_polygon,
                percent=percent,
                distance_to_agents=distance_to_agents,
                distance_to_polygon=distance_to_polygon,
                seed=seed,
                max_iterations=max_iterations,
                k=k,
            )
            st.text(f"Below should be a plot containing {len(samples)} agents")
            show_points(
                s_polygon=s_polygon,
                samples=samples,
                radius=distance_to_agents,
                obstacles=obstacles,
            )
        else:
            show_points(
                s_polygon=s_polygon,
                samples=[],
                radius=distance_to_agents,
                obstacles=obstacles,
            )


if __name__ == "__main__":
    main()
