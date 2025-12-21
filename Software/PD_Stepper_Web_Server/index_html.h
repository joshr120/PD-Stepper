const char index_html[] PROGMEM = R"rawliteral(




<!DOCTYPE HTML><html><head>
  <title>PD Stepper</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  
  <style>
  
  body {
    font-family: "Lato", sans-serif;
    background-color: #232324; 
    padding: 15px;
    width: 460px;
    margin:0 auto;
    color: white;
  }
  
  #inputs {
    border-radius: 20px;
    padding: 20px;
    margin: 0 auto;
    background-color: #2e2e2e; 
    width: 400px;
  }
  
  hr {
    width: 350px;
  }
  
  h2 {
    text-align: center;
  }
  
  h1 {
    margin-top: 0px;
    margin-bottom: 15px;
    font-size: 40px;
    text-align: center;
    text-decoration: ;
    color: #fc4903;
    font-family: Tahoma, sans-serif;
    font-weight: normal;
  }
  
  #middle-box {
    width: 320px;
    margin-left: auto;
    margin-right: auto;
    margin-bottom: 20px;
  }
  
  .drop-box {
    float: right;
    height: 25px;
    padding-left: 5px;
    padding-right: 5px;
    color: white;
    background-color: #2b2b2b;
    border-radius: 5px;
    border-style: solid;
    border-color: #919191;
    border-width: 1px;
  }
  
  
  #bottom-box {
    text-align: center;
  }
  
  .save-button {
    background-color: #fc4903;
    border: none;
    color: white;
    padding: 14px 30px;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    font-size: 16px;
    margin: 4px 2px;
    transition-duration: 0.4s;
    cursor: pointer;
    border-radius: 10px;
    font-size: 17px;
  }
  
    .pos-button {
    background-color: #fc4903;
    border: none;
    color: white;
    padding: 10px 14px;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    font-size: 16px;
    margin: 4px 2px;
    transition-duration: 0.4s;
    cursor: pointer;
    border-radius: 10px;
    
  }
  
  
  /* ----- CHECKBOX ------- */
  .container {
    display: inline;
    position: relative;
    padding-left: 35px;
    margin-bottom: 12px;
    margin-left: 160px;
    cursor: pointer;
    font-size: 22px;
    -webkit-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
  }
  #enabled1-text, #enabled2-text {
      padding-top:5px;
      display: inline-block;
      margin-right: 20px;
  }
  /* Hide the browser's default checkbox */
  .container input {
    position: absolute;
    opacity: 0;
    cursor: pointer;
    height: 0;
    width: 0;
  }

  /* Create a custom checkbox */
  .checkmark {
    position: absolute;
    top: 0;
    left: 0;
    height: 25px;
    width: 25px;
    background-color: #eee;
  }

  /* On mouse-over, add a grey background color */
  .container:hover input ~ .checkmark {
    background-color: #ccc;
  }

  /* When the checkbox is checked, add a blue background */
  .container input:checked ~ .checkmark {
    background-color: #fc4903;
  }

  /* Create the checkmark/indicator (hidden when not checked) */
  .checkmark:after {
    content: "";
    position: absolute;
    display: none;
  }

  /* Show the checkmark when checked */
  .container input:checked ~ .checkmark:after {
    display: block;
  }

  /* Style the checkmark/indicator */
  .container .checkmark:after {
    left: 9px;
    top: 5px;
    width: 5px;
    height: 10px;
    border: solid white;
    border-width: 0 3px 3px 0;
    -webkit-transform: rotate(45deg);
    -ms-transform: rotate(45deg);
    transform: rotate(45deg);
  }

  #powergood, #position, #status, #voltage, #stallguard{
    float: right;
  }
  
    .slider1 {
      width: 320px;
      height: 40px;
      accent-color: #fc4903;
   }


    /* Style the toggle switch container */
    .toggle-container {
      display: flex;
      flex-direction: column;
      align-items: center;
      margin-bottom: 20px; /* Adjust spacing between checkbox and slider */
    }

    /* Style the toggle switch */
    .toggle-switch {
      float: right;
      position: relative;
      display: inline-block;
      width: 50px;
      height: 24px;
    }

    /* Hide the default checkbox */
    .toggle-switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }

    /* The slider */
    .slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #ccc;
      -webkit-transition: .4s;
      transition: .4s;
      border-radius: 34px;
    }

    /* Rounded sliders */
    .slider.round {
      border-radius: 24px;
    }

    /* Slider styling when toggled */
    .slider:before {
      position: absolute;
      content: "";
      height: 16px;
      width: 16px;
      left: 4px;
      bottom: 4px;
      background-color: white;
      -webkit-transition: .4s;
      transition: .4s;
      border-radius: 50%;
    }

    input:checked + .slider {
      background-color: #fc4903;
    }

    input:focus + .slider {
      box-shadow: 0 0 1px #2196F3;
    }

    input:checked + .slider:before {
      -webkit-transform: translateX(26px);
      -ms-transform: translateX(26px);
      transform: translateX(26px);
    }
    
    img{
      width: 250px;
        display: block;
        margin-left: auto;
        margin-right: auto;
        margin-top: 20px;
    }

  
  </style>
  
</head><body>

  <div id="inputs">

    <h1>PD STEPPER</h1>
  <hr>
    <form action="/get">
      <h2>Live Stats</h2>
      <div id="middle-box">
        <span class="stat-labels">PD Voltage:</span>
        <span id="voltage">Loading</span>
        <br>
        <br>
        <span class="stat-labels">USB PD Status:</span>
        <span id="powergood">Loading</span>
        <br>
        <br>
        <span class="stat-labels">Driver Status:</span>
        <span id="status">Loading</span>
        <br>
        <br>
        <span class="stat-labels">Encoder Position:</span>
        <span id="position">Loading</span>
        <br>
        <br>
        <span class="stat-labels">Stall Guard:</span>
        <span id="stallguard">Loading</span>
      </div>
    </form>
 
    <hr>
  
    <form action="/save" method="post">
    <h2>Settings</h2>
      <div id="middle-box">
        <label for="enabled1" id="enabled1-text">Driver Enable:</label>
        <label class="container">
          <input %enabled1% type="checkbox" id="enabled1" name="enabled1" value="Enabled">
          <span class="checkmark"></span>
        </label>
        
        <br>
        <br>
        
        <label for="setvoltage">Voltage:</label>
        <select id="setvoltage" name="setvoltage" class="drop-box">
          <option value="5">5V</option>
          <option value="9">9V</option>
          <option value="12">12V</option>
          <option value="15">15V</option>
          <option value="20">20V</option>
        </select>
        <br>
        <br>

        <label for="microsteps">Microsteps:</label>
        <select id="microsteps" name="microsteps" class="drop-box">
          <option value="1">1</option>
          <option value="4">4</option>
          <option value="8">8</option>
          <option value="16">16</option>
          <option value="32">32</option>
          <option value="64">64</option>
          <option value="128">128</option>
          <option value="256">256</option>
        </select>
        <br>
        <br>

        <label for="current">Max Current:</label>
        <select id="current" name="current" class="drop-box">
          <option value="10">10&#37;</option>
          <option value="20">20&#37;</option>
          <option value="30">30&#37;</option>
          <option value="40">40&#37;</option>
          <option value="50">50&#37;</option>
          <option value="60">60&#37;</option>
          <option value="70">70&#37;</option>
          <option value="80">80&#37;</option>
          <option value="90">90&#37;</option>
          <option value="100">100&#37;</option>
        </select>
        <br>
        <br>

      <label for="stall_threshold">Stall Threshold:</label>
        <select id="stall_threshold" name="stall_threshold" class="drop-box">
          <option value="5">5</option>
          <option value="10">10</option>
          <option value="20">20</option>
          <option value="30">30</option>
          <option value="40">40</option>
          <option value="50">50</option>
          <option value="60">60</option>
          <option value="70">70</option>
          <option value="80">80</option>
          <option value="90">90</option>
          <option value="100">100</option>
          <option value="110">110</option>
          <option value="120">120</option>
          <option value="130">130</option>
          <option value="140">140</option>
          <option value="150">150</option>
        </select>
        <br>
        <br>
        <label for="standstill_mode">Standstill mode:</label>
        <select id="standstill_mode" name="standstill_mode" class="drop-box">
          <option value="NORMAL">Normal</option>
          <option value="FREEWHEELING">Freewheeling</option>
          <option value="BRAKING">Braking</option>
          <option value="STRONG_BRAKING">Strong-Braking</option>
        </select>
        
      </div>

      <div id="bottom-box">
        <input class="save-button" type="submit" value="Save">
      </div>
    </form>
    
    
    <br>
    <hr>
    
    <h2>Velocity Control</h2>
    <div id="middle-box">
      <label for="return">Snap Centre:</label>
      <label class="toggle-switch">
        <input type="checkbox" id="toggleReset" onclick="toggleReset()">
        <span class="slider round"></span>
      </label>
      <br>
      <input type='range' class='slider1' id='slider' min='-320' max='320' value='0' oninput='throttledUpdate()' onpointerup='checkReset()'>
   </div>   
   
   <hr>
    
   <h2>Position Control</h2>
   <div id="bottom-box">
    <button class="pos-button" type="submit" onclick="but1()"><<</button>
    <button class="pos-button" type="submit" onclick="but2()"><</button>
    <button class="pos-button" type="submit" onclick="but3()">></button>
    <button class="pos-button" type="submit" onclick="but4()">>></button>
   </div>
   
    <br>
  <hr>

   <img src="data:image/png;base64,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">
      
  </div>
  
  
</body></html>

<script>
  /* Auto fill dropdowns with saved values */ 
  var temp = "%microsteps%";
  var mySelect = document.getElementById('microsteps');
  for(var i, j = 0; i = mySelect.options[j]; j++) {
      if(i.value == temp) {
          mySelect.selectedIndex = j;
          break;
      }
  }
  
  var temp = "%voltage%";
  var mySelect = document.getElementById('setvoltage');
  for(var i, j = 0; i = mySelect.options[j]; j++) {
      if(i.value == temp) {
          mySelect.selectedIndex = j;
          break;
      }
  }
  
  var temp = "%current%";
  var mySelect = document.getElementById('current');
  for(var i, j = 0; i = mySelect.options[j]; j++) {
      if(i.value == temp) {
          mySelect.selectedIndex = j;
          break;
      }
  }
  
  var temp = "%stall_threshold%";
  var mySelect = document.getElementById('stall_threshold');
  for(var i, j = 0; i = mySelect.options[j]; j++) {
      if(i.value == temp) {
          mySelect.selectedIndex = j;
          break;
      }
  }
  
  var temp = "%standstill_mode%";
  var mySelect = document.getElementById('standstill_mode');
  for(var i, j = 0; i = mySelect.options[j]; j++) {
      if(i.value == temp) {
          mySelect.selectedIndex = j;
          break;
      }
  }
  
  /* Auto update stats without refresh */ 
  setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("voltage").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "/voltage", true);
    xhttp.send();
  }, 390 ) ;
  
  setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("powergood").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "/powergood", true);
    xhttp.send();
  }, 410 ) ;
  
  setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("status").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "/status", true);
    xhttp.send();
  }, 300 ) ;
  
  setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("position").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "/position", true);
    xhttp.send();
  }, 130 ) ;
  
  setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("stallguard").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "/stallguard", true);
    xhttp.send();
  }, 150 ) ;


  /* Throttle sending rate */
  let lastCall = 0;
  const throttleTimeout = 100; 
  
  function throttledUpdate() {
    const now = new Date().getTime();
    if (now - lastCall < throttleTimeout) return; // Skip if too soon
    lastCall = now;
    updateSlider();
  }
    
  
  /* Scripts for velocity toggle and slider */
  var resetEnabled = false;

  function updateSlider() {
    var sliderValue = document.getElementById('slider').value;
    var xhr = new XMLHttpRequest();
    xhr.open('POST', '/update', true);
    xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    xhr.send('slider=' + sliderValue);
  }

  function toggleReset() {
    var toggleCheckbox = document.getElementById('toggleReset');

    if (toggleCheckbox.checked) {
      // Reset slider to middle position if toggle is checked
      resetSlider();
    }

    resetEnabled = toggleCheckbox.checked;
  }

  function checkReset() {
    if (resetEnabled) {
      resetSlider(); // Reset slider if enabled
    }
  }

  function resetSlider() {
    document.getElementById('slider').value = 0; // Reset to middle position
    updateSlider(); // Trigger update
  }
  
  /* handle button presses */
  function but1() {
    var xhr = new XMLHttpRequest();
    xhr.open('POST', '/update', true);
    xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    xhr.send('positionControl=1');
  }
  
  function but2() {
    var xhr = new XMLHttpRequest();
    xhr.open('POST', '/update', true);
    xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    xhr.send('positionControl=2');
  }
  
  function but3() {
    var xhr = new XMLHttpRequest();
    xhr.open('POST', '/update', true);
    xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    xhr.send('positionControl=3');
  }
  
  function but4() {
    var xhr = new XMLHttpRequest();
    xhr.open('POST', '/update', true);
    xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    xhr.send('positionControl=4');
  }
  
</script>





)rawliteral";
