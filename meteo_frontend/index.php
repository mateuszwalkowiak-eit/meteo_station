<?php
ini_set('display_errors', 1); error_reporting(E_ALL); 
$servername = "localhost";
$dbname = "name";
$username = "user";
$password = "pass";

$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}
if(isset($_POST['daily'])) {
  $sql = "SELECT id, value1, value2, value3, value4, value5, reading_time FROM Sensor WHERE reading_time >= NOW() - INTERVAL 1 DAY order by reading_time desc";
}
elseif(isset($_POST['weekly'])) {
  $sql = "SELECT id, value1, value2, value3, value4, value5, reading_time FROM Sensor WHERE reading_time >= NOW() - INTERVAL 1 WEEK order by reading_time desc";
}

elseif(isset($_POST['monthly'])) {
  $sql = "SELECT id, value1, value2, value3, value4, value5, reading_time FROM Sensor WHERE reading_time >= NOW() - INTERVAL 1 MONTH order by reading_time desc";
}
else {
  $sql = "SELECT id, value1, value2, value3, value4, value5, reading_time FROM Sensor WHERE reading_time >= NOW() - INTERVAL 1 DAY order by reading_time desc";
}
$result = $conn->query($sql);
while ($data = $result->fetch_assoc()){
  $sensor_data[] = $data;
}

if(isset($_POST['daily'])) {
  $avg = "SELECT ROUND(AVG(value1),1) AS value1, ROUND(AVG(value2),1) AS value2, ROUND(AVG(value3),1) AS value3, ROUND(AVG(value4),1) AS value4, ROUND(AVG(value5),1) AS value5 FROM Sensor WHERE reading_time >=NOW() - INTERVAL 1 DAY order by reading_time desc";
}
elseif(isset($_POST['weekly'])) {
  $avg = "SELECT ROUND(AVG(value1),1) AS value1, ROUND(AVG(value2),1) AS value2, ROUND(AVG(value3),1) AS value3, ROUND(AVG(value4),1) AS value4, ROUND(AVG(value5),1) AS value5 FROM Sensor WHERE reading_time >= NOW() - INTERVAL 1 WEEK order by reading_time DESC";
}
elseif(isset($_POST['monthly'])) {
  $avg = "SELECT ROUND(AVG(value1),1) AS value1, ROUND(AVG(value2),1) AS value2, ROUND(AVG(value3),1) AS value3, ROUND(AVG(value4),1) AS value4, ROUND(AVG(value5),1) AS value5 FROM Sensor WHERE reading_time >= NOW() - INTERVAL 1 MONTH  order by reading_time desc";
}
else {
  $avg = "SELECT ROUND(AVG(value1),1) AS value1, ROUND(AVG(value2),1) AS value2, ROUND(AVG(value3),1) AS value3, ROUND(AVG(value4),1) AS value4, ROUND(AVG(value5),1) AS value5 FROM Sensor WHERE reading_time >=NOW() - INTERVAL 1 DAY order by reading_time desc";
}

$result2 = $conn->query($avg);

while ($wiersz = $result2->fetch_assoc()) {
  $avg1 = $wiersz["value1"];
  $avg2 = $wiersz["value2"];
  $avg3 = $wiersz["value3"];
}
if(isset($_POST['daily'])) {
  $min = "SELECT ROUND(MIN(CAST(value1 AS float)),1) AS value1,  MIN(CAST(value2 AS float)) AS value2, ROUND(MIN(CAST(value3 AS float)),1) AS value3, MIN(value4) AS value4, MIN(value5) AS value5, value6 FROM Sensor WHERE reading_time >=NOW() - INTERVAL 1 DAY order by reading_time desc";
}
elseif(isset($_POST['weekly'])) {
  $min = "SELECT ROUND(MIN(CAST(value1 AS float)),1) AS value1,  MIN(CAST(value2 AS float)) AS value2, ROUND(MIN(CAST(value3 AS float)),1) AS value3, MIN(value4) AS value4, MIN(value5) AS value5, value6 FROM Sensor WHERE reading_time >= NOW() - INTERVAL 1 MONTH  order by reading_time desc";
}
elseif(isset($_POST['monthly'])) {
  $min = "SELECT ROUND(MIN(CAST(value1 AS float)),1) AS value1,  MIN(CAST(value2 AS float)) AS value2, ROUND(MIN(CAST(value3 AS float)),1) AS value3, MIN(value4) AS value4, MIN(value5) AS value5, value6 FROM Sensor WHERE reading_time >= NOW() - INTERVAL 1 MONTH order by reading_time desc";
}
else {
  $min = "SELECT ROUND(MIN(CAST(value1 AS float)),1) AS value1,  MIN(CAST(value2 AS float)) AS value2, ROUND(MIN(CAST(value3 AS float)),1) AS value3, MIN(value4) AS value4, MIN(value5) AS value5, value6 FROM Sensor WHERE reading_time >=NOW() - INTERVAL 1 DAY  order by reading_time desc";
}
$result3 = $conn->query($min);
while ($wiersz = $result3->fetch_assoc()) {
  $min1 = $wiersz["value1"];
  $min2 = $wiersz["value2"];
  $min3 = $wiersz["value3"];
}
if(isset($_POST['daily'])) {
$max = "SELECT ROUND(MAX(CAST(value1 AS float)),1) AS value1, MAX(CAST(value2 AS float)) AS value2, ROUND(MAX(CAST(value3 AS float)),1) AS value3, MAX(value4) AS value4, MAX(value5) AS value5 FROM Sensor WHERE reading_time >=NOW() - INTERVAL 1 DAY order by reading_time desc";
}
elseif(isset($_POST['weekly'])) {
  $max = "SELECT ROUND(MAX(CAST(value1 AS float)),1) AS value1, MAX(CAST(value2 AS float)) AS value2, ROUND(MAX(CAST(value3 AS float)),1) AS value3, MAX(value4) AS value4, MAX(value5) AS value5 FROM Sensor WHERE reading_time >= NOW() - INTERVAL 1 WEEK order by reading_time desc";
}
elseif(isset($_POST['monthly'])) {
  $max = "SELECT ROUND(MAX(CAST(value1 AS float)),1) AS value1, MAX(CAST(value2 AS float)) AS value2, ROUND(MAX(CAST(value3 AS float)),1) AS value3, MAX(value4) AS value4, MAX(value5) AS value5 FROM Sensor WHERE reading_time >= NOW() - INTERVAL 1 MONTH order by reading_time desc";
}
else {
  $max = "SELECT ROUND(MAX(CAST(value1 AS float)),1) AS value1, MAX(CAST(value2 AS float)) AS value2, ROUND(MAX(CAST(value3 AS float)),1) AS value3, MAX(value4) AS value4, MAX(value5) AS value5 FROM Sensor WHERE reading_time >=NOW() - INTERVAL 1 DAY order by reading_time desc";
}
$result4 = $conn->query($max);
while ($wiersz = $result4->fetch_assoc()) {
  $max1 = $wiersz["value1"];
  $max2 = $wiersz["value2"];
  $max3 = $wiersz["value3"];
}

$now ="SELECT id, value1, value2, value3, value4, value5, reading_time FROM Sensor ORDER BY reading_time DESC LIMIT 1";
$result5 = $conn->query($now);
while ($wiersz = $result5->fetch_assoc()) {
  $now1 = $wiersz["value1"];
  $now2 = $wiersz["value2"];
  $now3 = $wiersz["value3"];
  $czas = $wiersz["reading_time"];
  $datetime = DateTime::createFromFormat('Y-m-d H:i:s', $czas);
  $czas1 = $datetime->format('Y-m-d H:i');
}
$sig = "SELECT value5, value6 FROM Sensor ORDER BY reading_time DESC LIMIT 1";

$result6 = $conn->query($sig);

while ($wiersz = $result6->fetch_assoc()) {
  $quality = $wiersz["value5"];
  $signal = $wiersz["value6"];
}

$readings_time = array_column($sensor_data, 'reading_time');

$i = 0;
foreach ($readings_time as $reading){
  if(isset($_POST['daily'])) {
    $readings_time[$i] = date("H:i", strtotime("$reading + 0 hours"));

  }
    elseif(isset($_POST['weekly'])) {
    $readings_time[$i] = date("Y-m-d H:i", strtotime("$reading + 0 hours"));
  }
    elseif(isset($_POST['monthly'])) {
      $readings_time[$i] = date("Y-m-d H:i", strtotime("$reading + 0 hours"));
    }
    else {
      $readings_time[$i] = date("H:i", strtotime("$reading + 0 hours"));
    }
    $i += 1;
}

$value1 = json_encode(array_reverse(array_column($sensor_data, 'value1')), JSON_NUMERIC_CHECK);
$value2 = json_encode(array_reverse(array_column($sensor_data, 'value2')), JSON_NUMERIC_CHECK);
$value3 = json_encode(array_reverse(array_column($sensor_data, 'value3')), JSON_NUMERIC_CHECK);
$value4 = json_encode(array_reverse(array_column($sensor_data, 'value4')), JSON_NUMERIC_CHECK);
$value5 = json_encode(array_reverse(array_column($sensor_data, 'value5')), JSON_NUMERIC_CHECK);
$reading_time = json_encode(array_reverse($readings_time), JSON_NUMERIC_CHECK);

$result->free();
$conn->close();
?>

<!DOCTYPE html>
<html>
<head>
<link rel="shortcut icon" href="favicon.ico">
<meta name="author" content="Autor">
<title> Stacja Meteo</title>
<link rel="stylesheet" href="global.css">
<meta name="viewport" content="width=device-width, initial-scale=1">
    <script src="https://code.highcharts.com/highcharts.js"></script>
  </head>
  <body>
    <h2>Stacja Meteorologiczna</h2>
    <nav class="table">
	<h4>Wybierz zakres pomiaru:</h4>
<form method="post" action="">
    <input type="submit" name="daily" value="dzienny">
    <input type="submit" name="weekly" value="tygodniowy">
    <input type="submit" name="monthly" value="miesięczny">
</form>
    </nav>
    <br>
    <h4 align=center>Podsumowanie:</h4>
    <table align=center>
  <tr>
  <th></th>  
  <th scope="col">Min.</th>
    <th scope="col">Średnia</th>
    <th scope="col">Max.</th>
    <th scope="col">Δ</th>
    <th scope="col">Aktualnie</th>
  </tr>
  <tr>
    <th scope="row">Temp.</th><td><?php echo $min1;?></td><td><?php echo $avg1; ?></td><td><?php echo $max1;?></td><td><?php echo $max1 - $min1?></td><td><?php echo $now1?></td><th scope="row">°C</th>
  </tr>
  <tr>
    <th scope="row">Wilgot.</th><td><?php echo $min2;?></td><td><?php echo $avg2; ?></td><td><?php echo $max2;?></td><td><?php echo $max2 - $min2?></td><td><?php echo $now2?></td><th scope="row">%</th>
</tr>
<tr>
  <th scope="row">Ciśn.</th><td><?php echo $min3;?></td><td><?php echo $avg3; ?></td><td><?php echo $max3;?></td><td><?php echo (round($max3 - $min3,1))?></td><td><?php echo $now3?></td><th scope="row">hPa</th>
</tr>
<tr>
<th scope="col" colspan="8">Raport smogowy</th></tr>
<tr><td colspan="8">    <?php
    if ($quality<=35 && $quality>=0) {
      echo "Znakomita jakość powietrza";
    }
    elseif ($quality<=75 && $quality>35) {
      echo "Dobra jakość powietrza";
    }
    elseif ($quality<=115 && $quality>75) {
      echo "Lekkie zanieczyszczenie powietrza";
    }
    elseif ($quality<=150 && $quality>115) {
      echo "Średnie zanieczyszczenie powietrza";
    }
    elseif ($quality<=250 && $quality>150) {
      echo "Mocne zanieczyszczenie powietrza";
    }
    elseif ($quality>250) {
      echo "Ogromne zanieczyszczenie powietrza";
    }
    ?></td></tr>
</table>
<h6 class="update">Wersja oprogramowania: v1.22<br><br>
  Sygnał sieci WiFi: <?php echo $signal;?> dBm<br>
Ostatnia aktualizacja: <?php echo $czas1?></h6>

    <br>
    <div id="chart-temperature" class="container"></div>
    <div id="chart-humidity" class="container"></div>
    <div id="chart-pressure" class="container"></div>
    <div id="chart-density" class="container"></div>
    <div id="chart-quality" class="container"></div>
<script>

var value1 = <?php echo $value1; ?>;
var value2 = <?php echo $value2; ?>;
var value3 = <?php echo $value3; ?>;
var value4 = <?php echo $value4; ?>;
var value5 = <?php echo $value5; ?>;
var reading_time = <?php echo $reading_time; ?>;

var chartT = new Highcharts.Chart({
  chart:{ renderTo : 'chart-temperature' },
  title: { text: 'Temperatura powietrza' },
  series: [{
  name: 'Temperatura (°C)',
    showInLegend: false,
    data: value1
  }],
  plotOptions: {
    line: { animation: true,
      dataLabels: { enabled: false }
    },
    series: { 
    color: '#059e8a',
     marker: {
                enabled: false
}
 },
  },
  xAxis: { 
    type: 'datetime',
    categories: reading_time,
      labels: {
            step: 12
        }
 },
  yAxis: {
    title: { text: 'Temperatura (°C)' }
  },
  credits: { enabled: false }
});

var chartH = new Highcharts.Chart({
  chart:{ renderTo:'chart-humidity' },
  title: { text: 'Wilgotność' },
  series: [{
  name: 'Wilgotność (%)',
    showInLegend: false,
    data: value2
  }],
  plotOptions: {
    line: { animation: true,
      dataLabels: { enabled: false }
    },
 series: {
            marker: {
                enabled: false
            }
        }
  },
  xAxis: {
    type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M' },
    categories: reading_time,
    labels: {
            step: 12
        }
  },
  yAxis: {
    title: { text: 'Wilgotność (%)' }
  },
  credits: { enabled: false }
});


var chartP = new Highcharts.Chart({
  chart:{ renderTo:'chart-pressure' },
  title: { text: 'Ciśnienie powietrza' },
  series: [{
  name: 'Ciśnienie (hPa)',
    showInLegend: false,
    data: value3
  }],
  plotOptions: {
    line: { animation: true,
      dataLabels: { enabled: false }
    },
    series: { color: '#18009c',
marker: {
                enabled: false
},
},
  },
  xAxis: {
    type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M' },
    categories: reading_time,
      labels: {
            step: 12
        }
  },
  yAxis: {
    title: { text: 'Ciśnienie (hPa)' }
  },
  credits: { enabled: false }
});

var chartD = new Highcharts.Chart({
  chart:{ renderTo:'chart-density' },
  title: { text: 'Poziom pyłu PM 2.5 - (test)' },
  series: [{
  name: 'Poziom PM 2.5 (µg/m³)',
    showInLegend: false,
    data: value4
  }],
  plotOptions: {
    line: { animation: true,
      dataLabels: { enabled: false }
    },
    series: { color: '#00FF00',
marker: {
                enabled: false
},
},
  },
  xAxis: {
    type: 'datetime',
    categories: reading_time,
      labels: {
            step: 12
        }
  },
  yAxis: {
    title: { text: 'Poziom pyłu PM 2.5 (µg/m³)' }
  },
  credits: { enabled: false }
});

var chartT = new Highcharts.Chart({
  chart:{ renderTo : 'chart-quality' },
  title: { text: 'Jakość powietrza - (test)' },
  series: [{
  name: 'Jakość powietrza (%)',
    showInLegend: false,
    data: value5
  }],
  plotOptions: {
    line: { animation: true,
      dataLabels: { enabled: false }
    },
    series: { color: '#FF00FF',
marker: {
                enabled: false
},
 },
  },
  xAxis: { 
    type: 'datetime',
    categories: reading_time,
      labels: {
            step: 12
        }
  },
  yAxis: {
    title: { text: 'Jakość powietrza (%)' }
  },
  credits: { enabled: false }
});
</script>
</body>
</html>
