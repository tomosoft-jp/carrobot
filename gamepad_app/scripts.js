var connection = new WebSocket('ws://' + '192.168.10.xx' + ':81/', ['arduino']);
connection.onopen = function () {
  connection.send('Connect ' + new Date());
};
connection.onerror = function (error) {
  console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) {
  console.log('Server: ', e.data);
};

var move = '#';
var speed = 30;
var AIN1 = '00';
var AIN2 = '00';
var BIN1 = '00';
var BIN2 = '00';
var PWMA = '00';
var PWMB = '00';

var moveflag = 0;  // 0:stop 1:top 2:bottom
var turnflag = 0; // 0:natural 1:left 2:right

function carmove(id) {
  console.log('carmove');

  switch (id) {
    case 'top':
      moveflag = 1;
      turnflag = 0;

      AIN1 = '00';
      AIN2 = '01';
      BIN1 = '00';
      BIN2 = '01';
      PWMA = speed;
      PWMB = speed;
      break;

    case 'left':
      turnflag = 1;

      PWMA = speed;
      PWMB = speed / 2;
      break;

    case 'center':
      moveflag = 0;

      AIN1 = '01';
      AIN2 = '00';
      BIN1 = '01';
      BIN2 = '00';
      PWMA = '00';
      PWMB = '00';
      break;

    case 'right':
      turnflag = 2;

      PWMA = speed / 2;
      PWMB = speed;
      break;

    case 'bottom':
      moveflag = 2;
      turnflag = 0;

      AIN1 = '01';
      AIN2 = '00';
      BIN1 = '01';
      BIN2 = '00';
      PWMA = speed;
      PWMB = speed;
      break;
  }
  move = '#' + AIN1 + AIN2 + PWMA + BIN1 + BIN2 + PWMB;
  console.log('move: ' + move);
  connection.send(move);
}
function carspeed(id)
{
  console.log('carspeed');
  if (id === "up") {
    speed = 60;
  } else {
    speed = 30;
  }
  if (moveflag !== 0) {
    if (turnflag === 0) {
      PWMA = speed;
      PWMB = speed;
    } else if (turnflag === 1) {
      PWMA = speed;
      PWMB = speed / 2;
    } else if (turnflag === 2) {
      PWMA = speed / 2;
      PWMB = speed;
    }
  }
  move = '#' + AIN1 + AIN2 + PWMA + BIN1 + BIN2 + PWMB;
  console.log('move: ' + move);
  connection.send(move);
}

function carconnect()
{
  connect = '%';
  console.log('connect: ' + connect);
  connection.send(connect);
}

