<?php
 /**
  *  web.php - Simple Web interface for talking to the coke machine lights
  *  using PHP because screw typing
  */
  $cokeaddress = "http://130.95.13.96/";
  $cokeurl = $cokeaddress;
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
			min-width: 1024px;
			background: #DDDDDD;
		}
		body {
			width: 1024px;
			min-width: 1024px;
			margin-left: auto;
			margin-right: auto;
			background: #FFFFFF;
		}
		.lights {
			width: 630px;
			height: 180px;
			padding: 0;
			margin: 0;
			font-size: 0%;
			float: left;
		}
		.led {
			width: 84px;
			height: 24px;
			margin: 0px;
			padding: 1px;
			background: #000000;
			display: inline-block;
		}
		.led:hover {
			padding: 0px;
			border: solid 1px #FF0000;
		}
		.lightcontrol {
			float: right;
			width: 394px;
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
	<div class="lightcontrol" id="lightcontrol">
		x: <input type="text" disabled value="" id="x"><br />
		y: <input type="text" disabled value="" id="y"><br />
		Red:<br />
		<input type="text" id="red" class="colour" data-slider="true" data-slider-theme="volume" data-slider-range="0,255" data-slider-step="1" /><br />
		Green:<br />
		<input type="text" id="green" class="colour" data-slider="true" data-slider-theme="volume" data-slider-range="0,255" data-slider-step="1" /><br />
		Blue:<br />
		<input type="text" id="blue" class="colour" data-slider="true" data-slider-theme="volume" data-slider-range="0,255" data-slider-step="1" /><br />
		<input type="button" id="setlights" value="Set Lights" />
	</div>
	<div class="brightness" style="display:none;" >
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
	
	// Sets Brightness
	/*$(".brightness").change(function() {
		console.log('<?=$cokeurl?>brightness?bright='+document.getElementById('bright').value);
		$.ajax({
			url: '<?=$cokeaddress?>brightness?bright='+document.getElementById('bright').value,
			dataType: "jsonp",
			crossDomain: true
		});
		
		return false;
	});*/
	
	$(".led").click(function() {
		console.log("hihi");
		// id contains info we need
		var id = this.id;
		console.log(id);
		var x = id.substring(3,4);
		console.log(x);
		var y = id.substring(5,6);
		console.log(y);
		$('#x').val(x);
		$('#y').val(y);
	});
	
	$(".colour").change(function() {
		setLED($('#x').val(), $('#y').val(), $('#red').val(), $('#green').val(),
			$('#blue').val());
		document.getElementById('lightcontrol').style.backgroundColor = "rgb(" + $('#red').val() +
			", " + $('#green').val() + ", " + $('#blue').val() + ")";
	});
	
	function getLights() {
		console.log('<?=$cokeurl?>get');
		$.ajax({
			url: '<?=$cokeaddress?>get',
			dataType: "jsonp",
			crossDomain: true,
			jsonp: false,
			jsonpCallback: "a",
			success: function(data) {
				//console.log(data.lights);
				lights = data.lights;
				for (var i = 0; i < 7; i++) {
					for (var j = 0; j < 6; j++) {
						var l = "led" + i + "x" + j
						var pos = i * 6 + j;
						//console.log(l);
						//console.log(pos);
						var hex = lights.substring(pos * 6, pos * 6 + 6);
						//console.log(hex);
						document.getElementById(l).style.backgroundColor =
							"#" + hex;
					}
				}
			}
		});
		
		return "true";
	}
	
	var hexDigits = new Array
		("0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"); 
	function hex(x) {
		return isNaN(x) ? "00" : hexDigits[(x - x % 16) / 16] + hexDigits[x % 16];
	}
	
	$("#setlights").click(function() {
		console.log("Set Lights");
		var lights = "";
		for (var i = 0; i < 7; i++) {
			for (var j = 0; j < 6; j++) {
				var l = "led" + i + "x" + j
				var pos = i * 6 + j;
				var colour = document.getElementById(l).style.backgroundColor;
				colour = colour.match(/^rgb\((\d+),\s*(\d+),\s*(\d+)\)$/);
				var h = hex(colour[1]) + hex(colour[2]) + hex(colour[3]);
				//console.log(l);
				//console.log(h);
				lights = lights + h;
			}
		}
		console.log(lights);
		$.ajax({
			url: '<?=$cokeaddress?>set?' + lights,
			dataType: "jsonp",
			crossDomain: true,
			jsonp: false,
			jsonpCallback: "a",
			success: function(data) {
				console.log("change!");
			}
		});
	});
	
	// Gets brightness
	/*function getBrightness() {
		console.log('<?=$cokeurl?>brightness);
		$.ajax({
			url: '<?=$cokeaddress?>brightness,
			dataType: "jsonp",
			crossDomain: true,
			success: function(response) {
				$(".brightness").value(response);
			}
		});
		
		return false;
	});*/
	
	function load() {
		getLights();
	}
	
	
	$(document).ready(function(){load();});
	</script>
</body>
</html>