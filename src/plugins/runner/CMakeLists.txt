# Search and/or reverse geocoding
add_subdirectory( hostip )
add_subdirectory( latlon )
add_subdirectory( local-osm-search )
add_subdirectory( localdatabase )
add_subdirectory( nominatim )

# Routing
add_subdirectory( gosmore )
add_subdirectory( mapquest )
add_subdirectory( monav )
add_subdirectory( openrouteservice )
add_subdirectory( routino )
add_subdirectory( yours )
# traveling-salesman works, but it is quite slow (tested version 1.0.3-RC1)
#add_subdirectory( traveling-salesman )

#Parsing
add_subdirectory( cache )
add_subdirectory( gpx )
add_subdirectory( kml )
add_subdirectory( osm )
add_subdirectory( pnt )
