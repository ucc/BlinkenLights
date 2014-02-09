<?php
 /**
  *  web.php - Simple Web interface for talking to the coke machine lights
  *  using PHP because screw typing
  */
  $cokeaddress = "http://130.95.13.96/";
?>
<!DOCTYPE html>
<html>
<head>
	<title>BlinkenLights</title>
	<script src="http://ajax.googleapis.com/ajax/libs/jquery/1.8.2/jquery.min.js"></script>
	
	<script src="js/simple-slider.js"></script>
	<link href="css/simple-slider.css" rel="stylesheet" type="text/css" />
	<link href="css/simple-slider-volume.css" rel="stylesheet" type="text/css" /> 
	
	<script type="text/javascript">
		function setLED(x, y, r, g, b)
		{
			var led = document.getElementById('led'+x+'x'+y);
			console.log('led'+x+'x'+y);
			led.style.backgroundColor = "rgb("+r+", "+g+", "+b+")";
		}
	</script>
	<style type="text/css">
		html {
			width: 100%;
			min-width: 480px;
			background: #DDDDDD;
		}
		body {
			width: 480px;
			margin-left: auto;
			margin-right: auto;
			background: #AAAAAA;
		}
		.led {
			width: 60px;
			height: 20px;
			margin: 0px;
			padding: 0;
			background: #000000;
			display: inline-block;
		}
		[class^=slider] { display: inline-block; margin-bottom: 30px; }
	</style>
</head>
<body>
	<div name="lights">
		<?php
		
		for ($i = 0; $i < 7; $i++) {
			for ($j = 0; $j < 6; $j++) {
				echo "<div class='led' id='led".$j."x".$i."'>&nbsp;</div>";
			}
			echo "<br />";
		}
		?>
	</div>
	<div class="brightness">
		Brightness:<br />
		<input type="text" id="bright" data-slider="true" data-slider-theme="volume" data-slider-range="0,255" data-slider-step="1" />
	</div>
	<script>
	$("[data-slider]")
	.each(function () {
		var input = $(this);
		$("<span>")
		.addClass("output")
		.insertAfter($(this));
	})
	.bind("slider:ready slider:changed", function (event, data) {
	  $(this)
		.nextAll(".output:first")
		.html(data.value.toFixed(0));
	});
	
	$(".brightness").change(function() {
		console.log('<?=$cokeaddress?>/brightness?bright='+document.getElementById('bright').value);
		$.ajax({
			url: '<?=$cokeaddress?>brightness?bright='+document.getElementById('bright').value,
			dataType: "jsonp",
			crossDomain: true
		});
		return false;
	});
		
	</script>
</body>
</html>