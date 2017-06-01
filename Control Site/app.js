window.onload = function() {

  // Get references to elements on the page.
  var form = document.getElementById('message-form');
  var messageField = document.getElementById('message');
  var distanceCell = document.getElementById('distance');
  var accelerometerCell = document.getElementById('accelerometer');
  var messagesList = document.getElementById('messages');
  var socketStatus = document.getElementById('status');
  var closeBtn = document.getElementById('close');

  var MoveForward = document.getElementById('Move_Forward');
  var MoveBack = document.getElementById('Move_Back');
  var MoveLeft = document.getElementById('Move_Left');  
  var MoveRight = document.getElementById('Move_Right');
  var Stop = document.getElementById('Stop');
  var distance_msg = "";
  var accelerometer_msg = "";
  var text = "Text";
  // Create a new WebSocket.
  var socket = new WebSocket('ws://192.168.43.150:81');

  // Handle any errors that occur.
  socket.onerror = function(error) {
    console.log('WebSocket Error: ' + error);
    return false;
  };


  // Show a connected message when the WebSocket is opened.
  socket.onopen = function(event) {
    socketStatus.innerHTML = 'Connected'// to: ' + event.currentTarget.URL;
    socketStatus.className = 'open';
    return false;
  };


  // Handle messages sent by the server.
  socket.onmessage = function(event) {
    var message = event.data;
    //distanceCell.innerHTML +=message;
    //messagesList.innerHTML += '<li class="sent"><span>Received</span></li>' + message;
    //messagesList.innerHTML += '<li class="sent"><span>Received</span></li>' + message;
    if (message == "F" | message == "B" | message == "R" | message == "L" | message == "S"){ //if equle to 1 then it F,B,R...
      messagesList.innerHTML += '<li class="sent"><span>Received</span></li>' + message;	
    }else if(message == "Connected"){
      distanceCell.innerHTML = '<li class="sent"><span>' +message + '</span></li>'; 
    }else if(message == "<"){
      distance_msg = '<li class="sent"><span>Distance:</span></li>';
    }else if(message == ">"){
      distanceCell.innerHTML = distance_msg;
      distance_msg="";
    }else if(message == "["){
      accelerometer_msg = '<li class="sent"><span>accelerometer:</span></li>';
    }else if(message == "]"){
      accelerometerCell.innerHTML = accelerometer_msg;
      accelerometer_msg="";	
    }else{
      if (distance_msg != ""){
        distance_msg += message;	
      }
      if (accelerometer_msg != ""){
        accelerometer_msg += message;
      }
    }
      
    return false;
  };


  // Show a disconnected message when the WebSocket is closed.
  socket.onclose = function(event) {
    socketStatus.innerHTML = 'Disconnected from WebSocket.';
    socketStatus.className = 'closed';
    return false;
  };


  // Send a message when the form is submitted.

  
  form.onsubmit = function(e) {
    e.preventDefault();

    // Retrieve the message from the textarea.
    var message = messageField.value;

    // Send the message through the WebSocket.
    socket.send(message);

    // Add the message to the messages list.
    messagesList.innerHTML += '<li class="sent"><span>Sent:</span></li>' + message;

    // Clear out the message field.
    messageField.value = '';

    return false;
  };
  
  MoveForward.onclick = function(c) {
    c.preventDefault();
    socket.send('F');
    messagesList.innerHTML = '<li class="sent"><span>Command</span></li>Move UP';	
    return false;
  };
  MoveBack.onclick = function(c) {
    c.preventDefault();
    socket.send('B');
    messagesList.innerHTML = '<li class="sent"><span>Command</span></li>Move Back';	
    return false;
  };
  MoveRight.onclick = function(c) {
    c.preventDefault();
    socket.send('R');
    messagesList.innerHTML = '<li class="sent"><span>Command</span></li>Move Right';	
    return false;
  };
  MoveLeft.onclick = function(c) {
    c.preventDefault();
    socket.send('L');
    messagesList.innerHTML = '<li class="sent"><span>Command</span></li>Move Left';	
    return false;
  }; 
  Stop.onclick = function(c) {
    c.preventDefault();
    socket.send('S');
    messagesList.innerHTML = '<li class="sent"><span>Command</span></li>STOP';	
    return false;
  };   
  // Close the WebSocket connection when the close button is clicked.
  closeBtn.onclick = function(e) {
    e.preventDefault();

    // Close the WebSocket.
    socket.close();

    return false;
  };

};
