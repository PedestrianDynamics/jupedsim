import ezdxf
from shapely import LineString
import matplotlib.pyplot as plt


def line_to_linestring(line):
    start_point = entity.dxf.start
    end_point = entity.dxf.end
    return LineString([(start_point[0], start_point[1]), (end_point[0], end_point[1])])


def plot_line_strings(line_strings):
    # Erstellen einer neuen Figur und eines Achsenobjekts
    fig, ax = plt.subplots()

    # Durchlaufen der LineString-Objekte
    for line_string in line_strings:
        # Extrahieren der Koordinaten des LineString-Objekts
        x, y = line_string.xy
        # Plotten der Linie
        ax.plot(x, y)

    # Anpassen der Achsenbegrenzungen
    ax.autoscale()

    # Anzeigen des Plots
    plt.show()


if __name__ == "__main__":
    # Pfad zur DXF-Datei
    file_path = "./SiB2023_entrance_jupedsim.dxf"
    lines = []
    # Öffnen der DXF-Datei
    doc = ezdxf.readfile(file_path)

    # Zugriff auf das Modell (Modelspace)
    msp = doc.modelspace()

    # Durchlaufen aller Entitäten im Modell
    for entity in msp:
        if entity.dxftype() == "LINE":
            start_point = entity.dxf.start
            end_point = entity.dxf.end
            print(f"Startpunkt: {start_point} , Endpunkt: {end_point}")
            lines.append(line_to_linestring(entity))
        else:
            print("entity type:", entity.dxftype())
            print()
    plot_line_strings(lines)