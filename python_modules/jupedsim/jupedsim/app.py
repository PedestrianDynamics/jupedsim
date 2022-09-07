import streamlit as st
import distribution
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
    polygon = [(0, 0), (10, 0), (10, 10), (0, 10)]
    radius = 0.3
    area = distribution.area_of_polygon(polygon)
    st.text('below should be a button')
    style = st.radio("how to choose number of agents?", ("density", "number"))
    if style == "density":
        agents = None
        density = st.slider("persons / m²", 0.1, 7.5)
    elif style == "number":
        agents = st.slider("agents", 1, round(area*5))
        density = None
    else:
        agents = density = 0
    button_clicked = st.button('distribute agents')

    if button_clicked:
        samples = distribution.create_random_points(polygon, agents, radius, radius, density=density)
        st.text('below should be a plot')
        show_points(polygon, samples, radius)


if __name__ =="__main__":
    main()