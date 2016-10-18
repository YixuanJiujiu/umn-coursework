var map;
var service;
var infoWindow;

var kh = { lat: 44.9745476, lng: -93.23223189999999 };
var bh = { lat: 44.9740787, lng: -93.23738839999999 };
var lasx = { lat: 45.0749486, lng: -93.05465959999998 };
var rafters = { lat: 45.054603, lng: -92.80490379999998};

function initMap() 
{
    // Construct map
    map = new google.maps.Map(document.getElementById('map'), { 
        center: {lat: 45.01246569999999, lng: -92.99188279999998},
        zoom: 11
    });
    
    // Construct info windows
    var khInfoWindow = new google.maps.InfoWindow({ 
        content: "Keller Hall: CSCI 4041, CSCI 5103, CSCI 5801"
    });
    var bhInfoWindow = new google.maps.InfoWindow({ 
        content: "Bruininks Hall: CSCI 4131"
    });
    var lasxInfoWindow = new google.maps.InfoWindow({ 
        content: "LasX Industries: Work" 
    });
    var raftersInfoWindow = new google.maps.InfoWindow({
        content: "Rafters Bar & Grill: Tip back a few" 
    });
    
    // Construct bouncing markers
    var khMarker = new google.maps.Marker({ 
        position: kh, 
        map: map, 
        animation: google.maps.Animation.BOUNCE 
    });
    var bhMarker = new google.maps.Marker({ 
        position: bh, 
        map: map,  
        animation: google.maps.Animation.BOUNCE 
    });
    var lasxMarker = new google.maps.Marker({ 
        position: lasx, 
        map: map, 
        animation: google.maps.Animation.BOUNCE 
    });
    var raftersMarker = new google.maps.Marker({
        position: rafters, 
        map: map,
        animation: google.maps.Animation.BOUNCE
    });
    
    // Add click listeners for info windows.
    khMarker.addListener('click', function(){ 
        khInfoWindow.open(map, khMarker) 
    });
    bhMarker.addListener('click', function(){ 
        bhInfoWindow.open(map, bhMarker) 
    });
    lasxMarker.addListener('click', function(){ 
        lasxInfoWindow.open(map, lasxMarker) 
    });
    raftersMarker.addListener('click', function(){
        raftersInfoWindow.open(map, raftersMarker)
    });
    
    // Add click event listener for looking up a location.
    var geocoder = new google.maps.Geocoder();
    document.getElementById('submit').addEventListener('click', function(){
        geocodeAddress(geocoder);
    });
    
    infoWindow = new google.maps.InfoWindow();
    service = new google.maps.places.PlacesService(map);
    
    // Add click event listener for finding nearby restaurants.
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
            console.error('Geocode was not successful for the following reason: ' + status);
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
    
    service.radarSearch(request, callback);
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
    var marker = new google.maps.Marker({
        map: map,
        position: place.geometry.location
    });
    google.maps.event.addListener(marker, 'click', function() {
        service.getDetails(place, function(result, status) {
            if (status !== google.maps.places.PlacesServiceStatus.OK) 
            {
                console.error('Showing info window failed for the following reason' + status);
                return;
            }
            infoWindow.setContent('<strong>' + result.name + '</strong><br>' + result.formatted_address);
            infoWindow.open(map, marker);
        });
    });
}

/* The function below inserts a temporary image in the dom tree, specifically
    in a table cell. The temporary image is displayed when a location is moused
    over. */
function mouseOver(event)
{
    var name = event.target.className;
    var target = event.target.parentNode;
    var tempImage = document.createElement("img");
    tempImage.setAttribute("id", "tempImage");
    
    if (name == "lasxLocation")
    {
        tempImage.src = "../calendar-images/lasx.jpeg";
        target.appendChild(tempImage);
    }
    
    if (name == "khLocation")
    {
        tempImage.src = "../calendar-images/kh0.jpg";
        target.appendChild(tempImage);
    }
    
    if (name == "bhLocation")
    {
        tempImage.src = "../calendar-images/sts0.jpg";
        target.appendChild(tempImage);
    }
    
    if (name == "raftersLocation")
    {
        tempImage.src = "../calendar-images/rafters.png";
        target.appendChild(tempImage);
    }
}

/* This function removes the temorary image created by mouseOver */
function mouseOut(event)
{
    var name = event.target.className;
    
    if (name == "lasxLocation" || name == "khLocation" || name == "bhLocation" || name == "raftersLocation")
    {
        var target = event.target.parentNode;
        target.removeChild(document.getElementById('tempImage'));
    }
}

document.addEventListener("mouseover", mouseOver, false);   
document.addEventListener("mouseout", mouseOut, false);
