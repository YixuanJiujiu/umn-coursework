<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhmtl1/DTD/xhtml1-transitional.dtd">

<html>
    <head>
        <meta charset = "utf-8">
        <title>Fall 2016 Schedule</title>
        <link rel = "stylesheet" type = "text/css" href = "style.css">
    </head>
    <body>
        <h1>My Calendar</h1>
	<div>
            <nav>
		<a href="calendar.php" class="button">Calendar</a>
                <a href="input.php" class="button">Input</a>
            </nav>
        </div>
        <div>
<?php
	if (!(file_exists('calendar.txt'))) 
	{
		echo '<p>Calendar has no events. Please use the input page to enter some events.</p>';
	}
	else
	{
		$log_file = fopen("calendar.txt", "r");

		$calendar_entries = array();
		while(!feof($log_file))
		{
			$entry_json = fgets($log_file);
			$entry = json_decode($entry_json, true);
			$entry['start_time'] = date("H:i", strtotime($entry['start_time']));
			$entry['end_time'] = date("H:i", strtotime($entry['end_time']));
			$calendar_entries[] = $entry;
		}	
		fclose($log_file);

		usort($calendar_entries, function($entryA, $entryB)
		{
			if ($entryA['start_time'] == $entryB['start_time']) return 0;
			return $entryA['start_time'] < $entryB['start_time'] ? -1 : 1;
		});
	}
?>
	<table> 
<?php
	$location_list = array();
	$mon_entries = array();
	$tue_entries = array();
	$wed_entries = array();
	$thu_entries = array();
	$fri_entries = array();
	foreach ($calendar_entries as $entry)
	{
		$entry_day = $entry['day'];
		if ($entry_day == 'Mon')
		{
			$mon_entries[] = $entry;
		}
		else if ($entry_day == 'Tue')
		{
			$tue_entries[] = $entry;
		}
		else if ($entry_day == 'Wed')
		{
			$wed_entries[] = $entry;
		}
		else if ($entry_day == 'Thu')
		{
			$thu_entries[] = $entry;
		}
		else if ($entry_day == 'Fri')
		{
			$fri_entries[] =  $entry;
		}
	}
	
	if (count($mon_entries) > 0)
	{
		echo('<tr><td>MON</td>');
		foreach($mon_entries as $mon_entry)
		{
			$mon_eventname = $mon_entry['event_name'];
			$mon_starttime = $mon_entry['start_time'];
			$mon_endtime = $mon_entry['end_time'];
			$mon_location = $mon_entry['location'];
			$location_list[] = $mon_location;
			echo '<td><p>' . htmlspecialchars($mon_starttime) . '-' . htmlspecialchars($mon_endtime) . '<br>' . htmlspecialchars($mon_eventname) . '-' . htmlspecialchars($mon_location) . '</p></td>';
		}
		echo('</tr>');
	}
	
	if (count($tue_entries) > 0)
	{
		echo('<tr><td>TUE</td>');
		foreach($tue_entries as $tue_entry)
		{
			$tue_eventname = $tue_entry['event_name'];
			$tue_starttime = $tue_entry['start_time'];
			$tue_endtime = $tue_entry['end_time'];
			$tue_location = $tue_entry['location'];
			$location_list[] = $tue_location;
			echo '<td><p>' . htmlspecialchars($tue_starttime) . '-' . htmlspecialchars($tue_endtime) . '<br>' . htmlspecialchars($tue_eventname) . '-' . htmlspecialchars($tue_location) . '</p></td>';
		}
		echo('</tr>');
	}

	if (count($wed_entries) > 0)
	{
		echo('<tr><td>WED</td>');
		foreach($wed_entries as $wed_entry)
		{
			$wed_eventname = $wed_entry['event_name'];
			$wed_starttime = $wed_entry['start_time'];
			$wed_endtime = $wed_entry['end_time'];
			$wed_location = $wed_entry['location'];
			$location_list[] = $wed_location;
			echo '<td><p>' . htmlspecialchars($wed_starttime) . '-' . htmlspecialchars($wed_endtime) . '<br>' . htmlspecialchars($wed_eventname) . '-' . htmlspecialchars($wed_location) . '</p></td>';
		}
		echo('</tr>');
	}

	if (count($thu_entries) > 0)
	{
		echo('<tr><td>THU</td>');
		foreach($thu_entries as $thu_entry)
		{
			$thu_eventname = $thu_entry['event_name'];
			$thu_starttime = $thu_entry['start_time'];
			$thu_endtime = $thu_entry['end_time'];
			$thu_location = $thu_entry['location'];
			$location_list[] = $thu_location;
			echo '<td><p>' . htmlspecialchars($thu_starttime) . '-' . htmlspecialchars($thu_endtime) . '<br>' . htmlspecialchars($thu_eventname) . '-' . htmlspecialchars($thu_location) . '</p></td>';
		}
		echo('</tr>');
	}

	if (count($fri_entries) > 0)
	{
		echo('<tr><td>FRI</td>');
		foreach($fri_entries as $fri_entry)
		{
			$fri_eventname = $fri_entry['event_name'];
			$fri_starttime = $fri_entry['start_time'];
			$fri_endtime = $fri_entry['end_time'];
			$fri_location = $fri_entry['location'];
			$location_list[] = $fri_location;
			echo '<td><p>' . htmlspecialchars($fri_starttime) . '-' . htmlspecialchars($fri_endtime) . '<br>' . htmlspecialchars($fri_eventname) . '-' . htmlspecialchars($fri_location) . '</p></td>';
		}
		echo('</tr>');
	}
	$location_list = array_unique($location_list, SORT_STRING);
	$location_list = json_encode($location_list);
?>
	</table>
        </div>
        <div id = "map"></div>
	<script type = "text/javascript">
                function initMap()
                {
                        // Construct map
                        map = new google.maps.Map(document.getElementById('map'), {
                                center: {lat: 45.01246569999999, lng: -92.99188279999998},
                                zoom: 11
                        });
                
			var locations = JSON.parse('<?= $location_list; ?>');
			var geocoder = new google.maps.Geocoder();

			for (index = 0; index < locations.length; index++)
			{
				var address = locations[index];
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
		}
        </script>
        <script async
            src = "https://maps.googleapis.com/maps/api/js?key=<YOUR_KEY_HERE>&libraries=places&callback=initMap">
        </script>
    </body>
</html>
    
