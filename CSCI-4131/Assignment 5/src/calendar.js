var map;
var service;
var infoWindow;
var kh = { lat: 44.9745476, lng: -93.23223189999999 };
var bh = { lat: 44.9740787, lng: -93.23738839999999 };
var lasx = { lat: 45.0749486, lng: -93.05465959999998 };

function initMap() 
{
    // Construct map
    map = new google.maps.Map(document.getElementById('map'), { 
        center: kh,
        zoom: 11
    });
    
    // Construct info windows
    var khInfoWindow = new google.maps.InfoWindow({ 
        content: "Keller Hall Biatch" } 
    );
    var bhInfoWindow = new google.maps.InfoWindow({ 
        content: "Bruiniks Hall Biatch" } 
    );
    var lasxInfoWindow = new google.maps.InfoWindow({ 
        content: "LasX Industries Biatch" } 
    );
    
    // Construct bouncing markers
    var khMarker = new google.maps.Marker({ 
        position: kh, 
        map: map, 
        draggable: true, 
        animation: google.maps.Animation.BOUNCE 
    });
    var bhMarker = new google.maps.Marker({ 
        position: bh, 
        map: map, 
        draggable: true, 
        animation: google.maps.Animation.BOUNCE 
    });
    var lasxMarker = new google.maps.Marker({ 
        position: lasx, 
        map: map, 
        draggable: true, 
        animation: google.maps.Animation.BOUNCE 
    });
    
    // Add listeners for info windows.
    khMarker.addListener('click', function(){ 
        khInfoWindow.open(map, khMarker) 
    });
    bhMarker.addListener('click', function(){ 
        bhInfoWindow.open(map, bhMarker) 
    });
    lasxMarker.addListener('click', function(){ 
        lasxInfoWindow.open(map, lasxMarker) 
    });
    
    // Add click event listener for looking up a location.
    var geocoder = new google.maps.Geocoder();
    document.getElementById('submit').addEventListener('click', function(){
        geocodeAddress(geocoder);
    });
    
    // Add click event listener for finding nearby restaurants.
    service = new google.maps.places.PlacesService(map);
    document.getElementById('find').addEventListener('click', function(){
        performSearch();
    });
}

function geocodeAddress(geocoder) 
{
    var address = document.getElementById('address').value;
    geocoder.geocode({'address': address}, function(results, status){
        if (status === 'OK') 
        {
            map.setCenter(results[0].geometry.location);
            var marker = new google.maps.Marker({
                map: map,
                position: results[0].geometry.location
            });
        } 
        else 
        {
            alert('Geocode was not successful for the following reason: ' + status);
        }
    });
}

function performSearch() 
{
    var rad = document.getElementById('search-radius').value;
    var request = {
        location: kh,
        radius: rad,
        type: ['restaurant']
    };
    
    service.nearbySearch(request, callback);
}

function callback(results, status) 
{
    if (status === google.maps.places.PlacesServiceStatus.OK)
    {
        for (var i = 0, result; result = results[i]; i++) 
        {
             addMarker(result);
        }
    }
}

function addMarker(place) {
    var placeLoc = place.geometry.location;
    var marker = new google.maps.Marker({
        map: map,
        position: place.geometry.location
    });
    
    google.maps.event.addListener(marker, 'click', function(){
        infowindow.setContent(place.name);
        infowindow.open(map, this);
    });
}