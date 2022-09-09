import streamlit as st
import distributions as distribution
import matplotlib.pyplot as plt


def show_points(polygon, samples, radius, obstacles=None):
    if obstacles is None:
        obstacles = []
    borders = distribution.get_borders(polygon)
    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1)
    for elem in samples:
        ax.add_patch(plt.Circle(radius=radius / 2, xy=elem, fill=False))
        ax.add_patch(plt.Circle(radius=0.1, xy=elem, color="r"))

    n = len(polygon)
    i = 0
    while True:
        following = (i + 1) % n
        x_value = [polygon[i][0], polygon[following][0]]
        y_value = [polygon[i][1], polygon[following][1]]
        plt.plot(x_value, y_value, color='blue')

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
            plt.plot(x_value, y_value, color='red')

            i += 1
            if following == 0:
                break

    plt.xlim(borders[0], borders[1])
    plt.ylim(borders[2], borders[3])
    plt.axis('equal')
    st.pyplot(fig)


def main():
    default_polygon = [(0.0, 0.0), (10.0, 0.0), (10.0, 10.0), (0.0, 10.0)]
    agents = density = 0

    with st.sidebar:
        a_r = st.slider("space among agents", 0.1, 2.0, value=0.3)
        w_d = st.slider("space between agents and walls", 0.1, 2.0, value=0.3)
        st.text("Polygon settings")
        corners = st.number_input("number of corner points in polygon", 3, value=len(default_polygon))
        col1, col2 = st.columns(2)
        # polygon
        x_values, y_values = [], []
        with col1:
            for i in range(corners):
                x_values.append(st.number_input(f"x value {i + 1}",
                                                value=default_polygon[i][0] if i < len(default_polygon) else 0.0,
                                                step=1.0))
        with col2:
            for i in range(corners):
                y_values.append(st.number_input(f"y value {i + 1}",
                                                value=default_polygon[i][1] if i < len(default_polygon) else 0.0,
                                                step=1.0))
        seed = st.number_input("set a seed. 0 = random seed", 0)
        if seed == 0:
            seed = None
        st.text("Obstacle settings")
        obstacle_count = st.number_input("number of obstacles", 0, value=0)
        obstacle_corners = []
        obstacle_values = []
        for i in range(obstacle_count):
            obstacle_corners.append(st.number_input(f"number of corner points in obstacle {i + 1}", 3, value=4))
        col3, col4 = st.columns(2)
        with col3:
            for i in range(obstacle_count):
                obstacle_values.append([])
                for j in range(obstacle_corners[i]):
                    obstacle_values[i].append([st.number_input(f"x value {j + 1} for obstacle{i + 1}",
                                                               value=0.0, step=1.0)])
        with col4:
            for i in range(obstacle_count):
                for j in range(obstacle_corners[i]):
                    obstacle_values[i][j].append(st.number_input(f"y value {j + 1} for obstacle{i + 1}",
                                                                 value=0.0, step=1.0))

        obstacles = []
        for i in range(obstacle_count):
            obstacles.append([])
            for j in range(obstacle_corners[i]):
                obstacles[i].append((obstacle_values[i][j][0], obstacle_values[i][j][1]))

    polygon = []
    for x, y in zip(x_values, y_values):
        polygon.append((x, y))
    area = distribution.area_of_polygon(polygon)
    distribution_type = st.radio("how to destribute agents?", ("place random", "place everywhere", "place in Circle"))

    if distribution_type == "place in Circle":
        circle_count = st.number_input("number of circles", 1)
        col1, col2, col3 = st.columns(3)
        min_values, max_values = [], []
        densities, agents = [], []
        with col1:
            mid_x = st.number_input("Middle -> x value")
            for i in range(circle_count):
                min_values.append(st.number_input(f"minimum radius for Circle {i + 1}"))
        with col2:
            mid_y = st.number_input("Middle -> y value")
            for i in range(circle_count):
                max_values.append(st.number_input(f"maximum radius for Circle {i + 1}"))
        with col3:
            for i in range(circle_count):
                style = (st.radio(f"how to choose number of agents for Circle {i}", ("density", "number")))
                if style == "density":
                    agents.append(None)
                    densities.append(st.slider("persons / m²", 0.1, 7.5, key=i), )
                elif style == "number":
                    agents.append(st.slider("agents", 1, round(area * 5), key=i))
                    densities.append(None)
        distributer = distribution.Distribution((mid_x, mid_y))
        for i in range(circle_count):
            distributer.create_circle(min_values[i], max_values[i], number=agents[i], density=densities[i])

        button_clicked = st.button('distribute agents')
        if button_clicked:
            samples = distributer.place_in_Polygon(polygon, a_r, w_d, obstacles=obstacles, seed=seed)
            st.text('below should be a plot')
            show_points(polygon, samples, a_r, obstacles=obstacles)

    if distribution_type == "place random":
        style = st.radio("how to choose number of agents?", ("density", "number"))
        if style == "density":
            agents = None
            density = st.slider("persons / m²", 0.1, 7.5)
        elif style == "number":
            agents = st.slider("agents", 1, round(area * 5))
            density = None

        button_clicked = st.button('distribute agents')

        if button_clicked:
            samples = distribution.create_random_points(polygon, agents, a_r, w_d, seed, obstacles, density)
            st.text('below should be a plot')
            show_points(polygon, samples, a_r, obstacles=obstacles)

    if distribution_type == "place everywhere":
        button_clicked = st.button('distribute agents')

        if button_clicked:
            samples = distribution.create_points_everywhere(polygon, a_r, w_d, obstacles=obstacles, seed=seed)
            st.text('below should be a plot')
            show_points(polygon, samples, a_r, obstacles=obstacles)


if __name__ == "__main__":
    main()
