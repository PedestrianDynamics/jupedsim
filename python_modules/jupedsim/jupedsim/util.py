import shapely

from jupedsim import GeometryBuilder


def build_jps_geometry(geo: shapely.GeometryCollection):
    geo_builder = GeometryBuilder()

    for obj in geo.geoms:
        if obj.geom_type != "Polygon":
            raise Exception(
                "Unexpected geometry type found in GeometryCollection: {obj.type}"
            )
        geo_builder.add_accessible_area(obj.exterior.coords[:-1])
        for hole in obj.interiors:
            geo_builder.exclude_from_accssible_area(hole.coords[:-1])
    return geo_builder.build()
