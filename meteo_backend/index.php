<?php
// dane bazy danych
$servername = "localhost";
$username = "user";
$password = "pass";
$dbname = "name";

date_default_timezone_set('Europe/Warsaw');
$lat = 51.886111;
$lng = 17.0125;

// Cache sunrise/sunset: max 1x / 12h
$cacheTtlSeconds = 12 * 60 * 60;
$dateToday = date('Y-m-d');
$dateTomorrow = date('Y-m-d', strtotime('+1 day'));
$cacheFile = sys_get_temp_dir() . '/sunrise_sunset_' . md5($lat . ',' . $lng) . '.json';

function http_get_json($url) {
	$ctx = stream_context_create([
		'http' => [
			'timeout' => 4,
			'ignore_errors' => true,
			'header' => "User-Agent: wclient/1.0\r\n",
		]
	]);
	$raw = @file_get_contents($url, false, $ctx);
	if ($raw === false) return null;
	$j = json_decode($raw, true);
	if (!is_array($j) || ($j['status'] ?? null) !== 'OK') return null;
	return $j;
}

$cache = null;
if (is_file($cacheFile)) {
	$raw = @file_get_contents($cacheFile);
	$tmp = $raw ? json_decode($raw, true) : null;
	if (is_array($tmp)) $cache = $tmp;
}

$cacheValid =
	is_array($cache) &&
	($cache['dateToday'] ?? '') === $dateToday &&
	($cache['dateTomorrow'] ?? '') === $dateTomorrow &&
	(time() - (int)($cache['fetchedAt'] ?? 0)) < $cacheTtlSeconds;

if ($cacheValid) {
	$data = $cache['today'];
	$data2 = $cache['tomorrow'];
} else {
	// Utwórz adresy URL API dla konkretnych dat (łatwiejsze cache’owanie)
	$url  = "https://api.sunrise-sunset.org/json?lat=$lat&lng=$lng&formatted=0&date=$dateToday";
	$url2 = "https://api.sunrise-sunset.org/json?lat=$lat&lng=$lng&formatted=0&date=$dateTomorrow";

	$data = http_get_json($url);
	$data2 = http_get_json($url2);

	// Zapisz cache tylko jeśli oba requesty się udały
	if (is_array($data) && is_array($data2)) {
		@file_put_contents($cacheFile, json_encode([
			'fetchedAt' => time(),
			'dateToday' => $dateToday,
			'dateTomorrow' => $dateTomorrow,
			'today' => $data,
			'tomorrow' => $data2,
		]));
	} elseif (is_array($cache)) {
		// fallback: jeśli API padło, a mamy jakikolwiek cache, użyj go
		$data = $cache['today'] ?? $data;
		$data2 = $cache['tomorrow'] ?? $data2;
	}
}

// Przetwórz odpowiedź JSON i pobierz godziny wschodu i zachodu słońca
// (zakładamy poprawne 'results' jeśli API/cache działa; w razie braku dalej poleci N/A w logice UI)
$sunrise_time = strtotime($data['results']['sunrise'] ?? 'now');
$sunset_time = strtotime($data['results']['sunset'] ?? 'now');
$sunrise_time2 = strtotime($data2['results']['sunrise'] ?? 'now');
$sunset_time2 = strtotime($data2['results']['sunset'] ?? 'now');

$current_time = time(); // Dodajemy aktualny czas w formacie timestamp
$current_hour = date("H:i", $current_time); // Aktualny czas w formacie H:i

// Sprawdź, czy słońce już zaszło, jeśli tak, to pobierz godzinę zachodu słońca z danych dla jutra
if ($sunset_time < $current_time) {
    $sunset_time = $sunset_time2;
}

// Sprawdź, czy słońce już wzeszło, jeśli tak, to pobierz godzinę wschodu słońca z danych dla jutra
if ($sunrise_time < $current_time) {
    $sunrise_time = $sunrise_time2;
}

// Oblicz różnicę czasu między aktualnym czasem a czasem wschodu/zachodu słońca
$time_diff_sunrise = $sunrise_time - $current_time;
$time_diff_sunset = $sunset_time - $current_time;

// Wyświetl godziny wschodu i zachodu słońca w formacie H:i
$sunrise_hour = date("H:i", $sunrise_time);
$sunset_hour = date("H:i", $sunset_time);

// Wyświetl różnicę czasu między aktualnym czasem a czasem wschodu/zachodu słońca w formacie H:i
$diff_sunset = gmdate("H:i", abs($time_diff_sunset));
$diff_sunrise = gmdate("H:i", abs($time_diff_sunrise));

// Określ porę dnia
if ($current_hour >= $sunrise_hour && $current_hour < $sunset_hour) {
    $pora_dnia = "day";
} else {
    $pora_dnia = "night";
}

// połączenie z bazą danych
$conn = new mysqli($servername, $username, $password, $dbname);

// sprawdzenie połączenia z bazą danych
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// zapytanie do bazy danych
$sql_sensor = "SELECT * FROM Sensor ORDER BY id DESC LIMIT 1";
$result = $conn->query($sql_sensor);

// pobranie wyników z bazy danych i zapisanie ich do tablicy
if ($result->num_rows > 0) {
    $row = $result->fetch_assoc();
    $values = array(
        "temperature" => is_null($row["value1"]) ? "N/A" : $row["value1"],
        "humidity" => is_null($row["value2"]) ? "N/A" : $row["value2"],
        "pressure" => is_null($row["value3"]) ? "N/A" : $row["value3"],
        "sunrise" => is_null($sunrise_hour) ? "N/A" : $sunrise_hour,
        "sunset" => is_null($sunset_hour) ? "N/A" : $sunset_hour,
        "sunrise_diff" => is_null($diff_sunrise) ? "N/A" : $diff_sunrise,
        "sunset_diff" => is_null($diff_sunset) ? "N/A" : $diff_sunset,
        "pora_dnia" => is_null($pora_dnia) ? "N/A" : $pora_dnia
    );
} else {
    $values = array(
        "temperature" => "N/A",
        "humidity" => "N/A",
        "pressure" => "N/A",
        "sunrise" => "N/A",
        "sunset" => "N/A"
    );
}


// zamiana tablicy na format JSON
$json = json_encode($values);

// wyświetlenie wyniku
echo $json;

// zamknięcie połączenia z bazą danych
$conn->close();
?>
