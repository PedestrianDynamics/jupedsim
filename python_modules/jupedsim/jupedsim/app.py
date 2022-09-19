import streamlit as st
import distributions
import matplotlib.pyplot as plt


def show_points(polygon, samples, radius, distributer=None):
    box = distributions.get_bounding_box(polygon)
    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1)
    for elem in samples:
        ax.add_patch(plt.Circle(radius=radius / 2, xy=elem, fill=False))
        ax.add_patch(plt.Circle(radius=0.1, xy=elem, color="r"))

    if distributer is not None:
        center = distributer.mid_point
        for circle in distributer.circles:
            ax.add_patch(plt.Circle(radius=circle[0], xy=center, fill=False))
            ax.add_patch(plt.Circle(radius=circle[1], xy=center, fill=False))

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

    plt.xlim(box[0][0], box[1][0])
    plt.ylim(box[0][1], box[1][1])
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

        polygon = []
    for x, y in zip(x_values, y_values):
        polygon.append((x, y))
    area = distributions.shply.Polygon(polygon).area
    distribution_type = st.radio("how to distribute agents?", ("place random", "place in Circle"))

    if distribution_type == "place in Circle":
        circle_count = st.number_input("number of circles", 1)
        col1, col2, col3 = st.columns(3)
        min_values, max_values = [], []
        densities, agents = [], []
        default_center = (5.0, 5.0)
        with col1:
            mid_x = st.number_input("Center -> x value", value=default_center[0], step=1.0)
            for i in range(circle_count):
                min_values.append(st.number_input(f"minimum radius for Circle {i + 1}", step=1.0))
        with col2:
            mid_y = st.number_input("Center -> y value", value=default_center[1], step=1.0)
            for i in range(circle_count):
                max_values.append(st.number_input(f"maximum radius for Circle {i + 1}", step=1.0))
        with col3:
            for i in range(circle_count):
                style = (st.radio(f"how to choose number of agents for Circle {i}", ("density", "number")))
                if style == "density":
                    agents.append(None)
                    densities.append(st.slider("persons / m²", 0.1, 7.5, key=i), )
                elif style == "number":
                    agents.append(st.slider("agents", 1, round(area * 5), key=i))
                    densities.append(None)
        distributer = distributions.Distribution((mid_x, mid_y))
        for i in range(circle_count):
            distributer.create_circle(min_values[i], max_values[i], number=agents[i], density=densities[i])

        button_clicked = st.button('distribute agents')
        if button_clicked:
            samples = distributer.place_in_Polygon(polygon, a_r, w_d, seed=seed)
            st.text('below should be a plot')
            show_points(polygon, samples, a_r, distributer)
        else:
            show_points(polygon, [], a_r, distributer)

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
            samples = distributions.create_random_points(polygon, agents, a_r, w_d, seed, density)
            st.text('below should be a plot')
            show_points(polygon, samples, a_r)
        else:
            show_points(polygon, [], a_r)


if __name__ == "__main__":
    main()
