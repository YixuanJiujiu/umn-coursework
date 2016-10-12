function initMap() 
{
    var kh = { lat: 44.9745476, lng: -93.23223189999999 };
    var bh = { lat: 44.9740787, lng: -93.23738839999999 };
    var lasx = { lat: 45.0749486, lng: -93.05465959999998 };
    
    var map = new google.maps.Map(document.getElementById("map"), { 
        center: { lat: 45.00607670000001, lng: -93.15661069999999 },
        zoom: 11
    });
    
    var khInfoWindow = new google.maps.InfoWindow( { content: "Keller Hall Biatch" } );
    var bhInfoWindow = new google.maps.InfoWindow( { content: "Bruiniks Hall Biatch" } );
    var lasxInfoWindow = new google.maps.InfoWindow( { content: "LasX Industries Biatch" } );
    
    var khMarker = new google.maps.Marker( { position: kh, map: map, draggable: true, animation: google.maps.Animation.DROP } );
    var bhMarker = new google.maps.Marker( { position: bh, map: map, draggable: true, animation: google.maps.Animation.DROP } );
    var lasxMarker = new google.maps.Marker( { position: lasx, map: map, draggable: true, animation: google.maps.Animation.DROP } );
    
    khMarker.addListener('animation_changed', function() { bounceMarker(khMarker) } );
    bhMarker.addListener('animation_changed', function() { bounceMarker(bhMarker) } );
    lasxMarker.addListener('animation_changed', function() { bounceMarker(lasxMarker) } );
    
    khMarker.addListener('click', function() { khInfoWindow.open(map, khMarker) } );
    bhMarker.addListener('click', function() { bhInfoWindow.open(map, bhMarker) } );
    lasxMarker.addListener('click', function() { lasxInfoWindow.open(map, lasxMarker) } );
}

function bounceMarker(marker)
{
    if (marker.getAnimation() !== null) { marker.setAnimation(null); } 
    else { marker.setAnimation(google.maps.Animation.BOUNCE); }
}

window.onload = displayanimation;