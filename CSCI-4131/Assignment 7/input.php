<?php
	// if the form has not been submitted yet, set default values for initial page load
	if ($_SERVER['REQUEST_METHOD'] != 'POST')
	{
		if (!isset($eventname)) { $eventname = ''; }
		if (!isset($starttime)) { $starttime = '--:--' . ' --'; }
		if (!isset($endtime)) { $endtime = '--:--' . ' --'; }
		if (!isset($location)) { $location = ''; }
		if (!isset($day)) { $day = ''; }
	}
	else
	{
		// construct object containing submitted data
		$event_info = array();
		$event_info['event_name'] = filter_input(INPUT_POST, 'eventname');
		$event_info['start_time'] = filter_input(INPUT_POST, 'starttime');
		$event_info['end_time'] = filter_input(INPUT_POST, 'endtime');
		$event_info['location'] = filter_input(INPUT_POST, 'location');
		$event_info['day'] = filter_input(INPUT_POST, 'day');

		// write submitted data to logging file w/ JSON encoding
		$log_file = fopen("calendar.txt", "ab") or die(print_r(error_get_last(), TRUE));
		$info_json = json_encode($event_info);
		fwrite($log_file, $info_json . "\n");
		fclose("calendar.txt");

		// redirect to calendar.php using header function
		header('Location: http://www-users.cselabs.umn.edu/~pungx013/calendar.php');	
	}
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhmtl1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhmtl" xml:lang="en" lang="en">
	<head>
		<meta charset = "utf-8">
		<title>Form Input</title>
		<link rel = "stylesheet" type = "text/css" href = "style.css">
		<script>
			function validateForm()
			{
				var name = document.forms["eventForm"]["eventname"].value;
				var start = document.forms["eventForm"]["starttime"].value;
				var end = document.forms["eventForm"]["endtime"].value;
				var loc = document.forms["eventForm"]["location"].value;
				var day = document.forms["eventForm"]["day"].value;
				if (name == null || name == "" || start == null || start == "" || start == "--:-- --" || end == null || end == "" || end == "--:-- --" || loc == null || loc == "")
				{
					alert("All fields must be filled out entirely");
					return false;
				}
				if (day == "Sat" || day == "Sun")
				{
					alert("Events on saturdays and sundays are not supported by this calendar");
					return false;
				}
			}
		</script>
	</head>
	<body>
		<h1>Calendar Input</h1>
		<div>
			<nav>
				<a href="calendar.php" class="button">Calendar</a>
		                <a href="input.php" class="button">Input</a>
			</nav>
		</div>
		<div>
			<p>Please provide a vlaue for Event Name. </p>
			<p>Please enter a value for Event Start Time.</p>
			<p>Please enter a value for Event End Time.</p>
			<p>Please enter a value for Event Location.</p>
			<form name = "eventForm" method = "post">
				<p><label>Event Name
					<input name = "eventname" type = "text" value = "<?php echo htmlspecialchars($eventname); ?>">
				</label></p>				<p><label>Start Time
					<input name = "starttime" type = "time" value = "<?php echo htmlspecialchars($starttime); ?>">
				</label></p>
				<p><label>End Time
					<input name = "endtime" type = "time" value = "<?php echo htmlspecialchars($endtime); ?>">
				</label></p>
				<p><label>Location
					<input name = "location" type = "text" value = "<?php echo htmlspecialchars($location); ?>">
				</label></p>
				<p><label>Day of the week
					<select name = "day" value = "<?php echo htmlspecialchars($day); ?>">
						<option>Mon</option>
						<option>Tue</option>
						<option>Wed</option>
						<option>Thu</option>
						<option>Fri</option>
						<option>Sat</option>
						<option>Sun</option>
					</select>
				</label</p>
				<p>
					<input name = "delete" type = "submit" value = "Clear">
					<input type = "submit" value = "Submit" onclick = "return validateForm();"/>

					<?php
						if (isset($_POST['delete'])) 
						{
							unlink('calendar.txt');
						}
					?>
				</p>
			</form>
		</div>
	</body>
</html>
