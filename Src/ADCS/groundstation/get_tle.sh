curl -s -c cookies.txt -b cookies.txt https://for-testing-only.space-track.org/ajaxauth/login -d "identity=jacobtkeio@gmail.com&password=AGSSS!25AGSSS!25" &>/dev/null
curl -s --cookie cookies.txt "https://for-testing-only.space-track.org/basicspacedata/query/class/tle_latest/NORAD_CAT_ID/25544/orderby/EPOCH%20asc/limit/1/format/tle/emptyresult/show"
# TODO: replace 25544 with REALOP-1's NORAD_CAT_ID
