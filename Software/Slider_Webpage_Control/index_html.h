
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Slider Position</title>
  <style>
  
  body {
    background-color: #383838;
    color: white;
  font-family: Arial, Helvetica, sans-serif;
  }

  .slider1 {
      width: 40%;
      height: 40px;
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
      position: relative;
      display: inline-block;
      width: 60px;
      height: 34px;
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
      border-radius: 34px;
    }

    /* Slider styling when toggled */
    .slider:before {
      position: absolute;
      content: "";
      height: 26px;
      width: 26px;
      left: 4px;
      bottom: 4px;
      background-color: white;
      -webkit-transition: .4s;
      transition: .4s;
      border-radius: 50%;
    }

    input:checked + .slider {
      background-color: #2196F3;
    }

    input:focus + .slider {
      box-shadow: 0 0 1px #2196F3;
    }

    input:checked + .slider:before {
      -webkit-transform: translateX(26px);
      -ms-transform: translateX(26px);
      transform: translateX(26px);
    }
  </style>
</head>
<body>
  <div class="toggle-container">
  <h2>Stepper Control</h2>
    <label class="toggle-switch">
      <input type="checkbox" id="toggleReset" onclick="toggleReset()">
      <span class="slider round"></span>
    </label>
    <input type='range' class='slider1' id='slider' min='-100' max='100' value='0' oninput='updateSlider()' onpointerup='checkReset()'>
  </div>

  <script>
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
  </script>
</body>
</html>
)rawliteral";



/////////////////////////////////////
////  Better looking but slow for some reason
////////////////////////////////////


//const char index_html[] PROGMEM = R"rawliteral(
//<!DOCTYPE html>
//<html>
//<head>
//  <title>Slider Position</title>
//  <style>
//    /* Style the toggle switch container */
//    .toggle-container {
//      display: flex;
//      flex-direction: column;
//      align-items: center;
//      margin-bottom: 20px; /* Adjust spacing between checkbox and slider */
//    }
//
//    /* Style the toggle switch */
//    .toggle-switch {
//      position: relative;
//      display: inline-block;
//      width: 60px;
//      height: 34px;
//    }
//
//    /* Hide the default checkbox */
//    .toggle-switch input {
//      opacity: 0;
//      width: 0;
//      height: 0;
//    }
//
//    /* The slider */
//    .slider {
//      position: absolute;
//      cursor: pointer;
//      top: 0;
//      left: 0;
//      right: 0;
//      bottom: 0;
//      background-color: #ccc;
//      -webkit-transition: .4s;
//      transition: .4s;
//      border-radius: 34px;
//    }
//
//    /* Rounded sliders */
//    .slider.round {
//      border-radius: 34px;
//    }
//
//    /* Slider styling when toggled */
//    .slider:before {
//      position: absolute;
//      content: "";
//      height: 26px;
//      width: 26px;
//      left: 4px;
//      bottom: 4px;
//      background-color: white;
//      -webkit-transition: .4s;
//      transition: .4s;
//      border-radius: 50%;
//    }
//
//    input:checked + .slider {
//      background-color: #2196F3;
//    }
//
//    input:focus + .slider {
//      box-shadow: 0 0 1px #2196F3;
//    }
//
//    input:checked + .slider:before {
//      -webkit-transform: translateX(26px);
//      -ms-transform: translateX(26px);
//      transform: translateX(26px);
//    }
//  </style>
//</head>
//<body>
//  <div class="toggle-container">
//    <label class="toggle-switch">
//      <input type="checkbox" id="toggleReset" onclick="toggleReset()">
//      <span class="slider round"></span>
//    </label>
//    <input type='range' id='slider' min='-100' max='100' value='0' oninput='updateSlider()' onpointerup='checkReset()'>
//  </div>
//
//  <script>
//    var resetEnabled = false;
//
//    function updateSlider() {
//      var sliderValue = document.getElementById('slider').value;
//      var xhr = new XMLHttpRequest();
//      xhr.open('POST', '/update', true);
//      xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
//      xhr.send('slider=' + sliderValue);
//    }
//
//    function toggleReset() {
//      var toggleCheckbox = document.getElementById('toggleReset');
//      
//      if (toggleCheckbox.checked) {
//        // Reset slider to middle position if toggle is checked
//        resetSlider();
//      }
//
//      resetEnabled = toggleCheckbox.checked;
//    }
//
//    function checkReset() {
//      if (resetEnabled) {
//        resetSlider(); // Reset slider if enabled
//      }
//    }
//
//    function resetSlider() {
//      document.getElementById('slider').value = 0; // Reset to middle position
//      updateSlider(); // Trigger update
//    }
//  </script>
//</body>
//</html>
//)rawliteral";
