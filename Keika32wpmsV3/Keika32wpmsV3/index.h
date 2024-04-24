const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<head>
    <style>
        #header {
            min-height: 20px;
            background-color: #98ccf7;
            font-family:tahoma;
        }

        #menu {
            min-height: 20px;
            margin-top: 1%;
            background-color: #beb9b9;
            font-family:tahoma;
        }

        #body {
            min-height: 200px;
            margin-top: 1%;
        }

        #footer {
            min-height: 20px;
            margin-top: 1%;
            background-color: #98ccf7;
            font-family:tahoma;
        }

        #header,
        #menu,
        #body,
        #footer {
            margin-left: 10%;
            margin-right: 10%;
            box-shadow: 3px 5px 7px #9dccf8;
            border: 1px solid black;
        }

        @viewport {
            zoom: 1.0;
            width: extend-to-zoom;
        }

        @-ms-viewport {
            width: extend-to-zoom;
            zoom: 1.0;
        }
    </style>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Wemos KEIKA Web Server DEMO V0.1</title>
</head>

<style>
.card{
    max-width: 400px;
     min-height: 250px;
     background: #02b875;
     padding: 30px;
     box-sizing: border-box;
     color: #FFF;
     margin:20px;
     box-shadow: 0px 2px 18px -4px rgba(0,0,0,0.75);
}
</style>
<body>

<div class="card">
  <h4>The ESP32 Update web page without refresh</h4><br>
  <h1>Sensor Value:<span id="ADCValue">0</span></h1><br>
  <h1>Sensor Value:<span id="ADCValue">0</span></h1><br>
  <h1>Sensor Value:<span id="ADCValue">0</span></h1><br>
  <br><a href="https://circuits4you.com">Circuits4you.com</a>
</div>
</a><a href="/test.pdf"><button class="button"
style="background-color: #195B6A; border: none; color: white; padding: 16px 40px ; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; ">MANUAL DE USO
</button></a></p>
<script>

setInterval(function() {
  // Call a function repetatively with 2 Second interval
  getData();
}, 2000); //2000mSeconds update rate

function getData() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ADCValue").innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", "readADC", true);
  xhttp.send();
}
</script>
</body>
</html>
)=====";
