import streamlit as st
import distributions
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
        col1, col2 = st.columns(2)
        corners = st.number_input("number of corner points in polygon", 3, value=4)
        x_values, y_values = [], []
        with col1:
            for i in range(corners):
                x_values.append(st.number_input(f"x value {i+1}",
                                                value=default_polygon[i][0] if i < len(default_polygon) else 0))
        with col2:
            for i in range(corners):
                y_values.append(st.number_input(f"y value {i+1}",
                                                value=default_polygon[i][1] if i < len(default_polygon) else 0))

    polygon = []
    for x, y in zip(x_values, y_values):
        polygon.append((x, y))
    area = distribution.area_of_polygon(polygon)
    distribution_type = st.radio("how to destribute agents?", ("place random","place everywhere"))

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
            samples = distribution.create_random_points(polygon, agents, a_r, w_d, density=density)
            st.text('below should be a plot')
            show_points(polygon, samples, a_r)

    if distribution_type == "place everywhere":
        button_clicked = st.button('distribute agents')

        if button_clicked:
            samples = distribution.create_points_everywhere(polygon, a_r, w_d)
            st.text('below should be a plot')
            show_points(polygon, samples, a_r)


if __name__ == "__main__":
    main()
