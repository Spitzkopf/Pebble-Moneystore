var config = {};

var defaultIfNan = function(value, def)
{
  return isNaN(value) ? def : value;
};

var loadConfig = function()
{
  console.log("loading config");
  config.celsius = parseInt(localStorage.getItem("celsius"));
  config.celsius = defaultIfNan(config.celsius, 1);
  config.bt_vibe = parseInt(localStorage.getItem("bt_vibe"));
  config.bt_vibe = defaultIfNan(config.bt_vibe, 1);
  config.hour_vibe = parseInt(localStorage.getItem("hour_vibe"));
  config.hour_vibe = defaultIfNan(config.hour_vibe, 1);
  console.log("config loaded");
};

var saveConfig = function()
{
  console.log("saving config");
  localStorage.setItem("celsius", config.celsius);  
  localStorage.setItem("bt_vibe", config.bt_vibe); 
  localStorage.setItem("hour_vibe", config.hour_vibe); 
};

var kelvinToCelsius = function(kelvin) {
  return Math.round(kelvin - 273.15);
};

var kelvinToFarenheit = function(kelvin) {
  return Math.round((kelvin - 273.15) * 1.8000 + 32.00);
};

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  
  xhr.open(type, url);
    xhr.send();
};
    
var locationSuccess = function(pos) {
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' + pos.coords.latitude + '&lon=' + pos.coords.longitude;
  
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      // Temperature in Kelvin requires adjustment
      var temperature = config.celsius ? kelvinToCelsius(json.main.temp) : kelvinToFarenheit(json.main.temp);
      console.log('Temperature is ' + temperature);

      // Conditions
      var conditions = json.weather[0].main;      
      console.log('Conditions are ' + conditions);
      
      var dictionary = {
        'KEY_TEMPERATURE': temperature,
        'KEY_CONDITIONS': conditions
      };
      
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log('Weather info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending weather info to Pebble!');
        });
    });
};

var locationError = function(err) {
  console.log('Error requesting location!');
};

var getWeather = function() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
};

Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
    loadConfig();
    getWeather();
  }
);

Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getWeather();
  }                     
);

Pebble.addEventListener('showConfiguration', function(e){
  Pebble.openURL('https://still-fjord-3522.herokuapp.com/?conf=' + encodeURIComponent(JSON.stringify(config)));
  //Pebble.openURL('http://10.0.0.11:5000/?conf=' + encodeURIComponent(JSON.stringify(config)));
});

Pebble.addEventListener('webviewclosed', function(e){
  var tempConfig = JSON.parse(decodeURIComponent(e.response));
  console.log(JSON.stringify(config));
  
  if (tempConfig)
  {
    config = tempConfig;
    saveConfig();
    if (config.celsius !== undefined)
    {
      getWeather();
    }
    
    Pebble.sendAppMessage({
      "KEY_CELSIUS":parseInt(config.celsius), 
      "KEY_BTVIBE":parseInt(config.bt_vibe), 
      "KEY_HOURVIBE":parseInt(config.hour_vibe), 
    }); 
  } 
});