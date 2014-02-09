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
			min-width: 480px;
			margin-left: auto;
			margin-right: auto;
			background: #FFFFFF;
		}
		.lights {
			width: 210px;
			height: 60px;
			padding: 0;
			margin: 0;
			font-size: 0%;
			float: left;
		}
		.led {
			width: 28px;
			height: 8px;
			margin: 0px;
			padding: 1px;
			background: #000000;
			display: inline-block;
		}
		.led:hover {
			padding: 0px;
			background: #0000FF;
			border: solid 1px #FF0000;
		}
		.lightcontrol {
			float: right;
			width: 250px;
			padding: 0;
			margin: 0;
			background: #0000FF;
		}
		.brightness{
			clear: both;
		}
		[class^=slider] {
			display: inline-block;
		}
	</style>
</head>
<body>
	<div class="lights">
		<?php
		
		for ($i = 0; $i < 6; $i++) {
			for ($j = 0; $j < 7; $j++) {
				echo "<div class='led' id='led".$j."x".$i."'>&nbsp;</div>";
			}
			echo "<br />\r\n";
		}
		
		?>
	</div>
	<div class="lightcontrol">
		x: <input type="text" disabled value="" id="x"><br />
		y: <input type="text" disabled value="" id="y"><br />
		Red: <input type="text" value="" id="red"><br />
		Green: <input type="text" value="" id="green"><br />
		Blue: <input type="text" value="" id="blue"><br />
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
		
		return false;
	});
	</script>
</body>
</html>