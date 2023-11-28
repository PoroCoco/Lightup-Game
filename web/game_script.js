const myTimeout = setTimeout(start, 500);

/* ******************** global variables ******************** */
var canvas = document.getElementById('gamedraw');
var ctx = canvas.getContext('2d');
var g;
var ost = new Audio('textures/ost.mp3')
var won = new Audio('textures/orb.wav');
ost.volume=0.4;
won.volume=0.4;
const S_BLANK = 0;        
const S_LIGHTBULB = 1;    
const S_MARK = 2;         
const S_BLACK = 8;        
const S_BLACK0 = S_BLACK; 
const S_BLACK1 = 9;           
const S_BLACK2 = 10;           
const S_BLACK3 = 11;           
const S_BLACK4 = 12;           
const S_BLACKU = 13; 
const F_LIGHTED = 16;
const F_ERROR = 32; 

// load the images
var lightbulb = new Image();
lightbulb.src = "textures/lightbulb.png";

var lightbulb_error = new Image();
lightbulb_error.src = "textures/lightbulb_error.png";

var blank = new Image();
blank.src = "textures/blank.png";

var blank_lighted = new Image();
blank_lighted.src = "textures/blank_lighted.png";

var mark = new Image();
mark.src = "textures/mark.png";

var wall_error = new Image();
wall_error.src = "textures/wall_error.png";

var wall = new Image();
wall.src = "textures/wall.png";


/* ******************** register events ******************** */
canvas.addEventListener('click', canvasLeftClick);        // left click event
canvas.addEventListener('contextmenu', canvasRightClick); // right click event
window.addEventListener('resize', resizeCanvas);      // windowsResize event
window.addEventListener('focus', playmusic);        // focus on the web windows


function start() {
    g=Module._new_default();
    drawGame(g);
}

function playmusic(){
    ost.play();
}

function win(){
    if(Module._is_over(g)){
        won.play();
        var text = document.getElementById('winningText');
        text.style.display = "block";
    }else{
        var text = document.getElementById('winningText');
        text.style.display = "none";
    }
}

function drawGame(g){
    var width = canvas.width;
    var height = canvas.height;
    var nb_rows = Module._nb_rows(g);
    var nb_cols = Module._nb_cols(g);
    var squareX = width/nb_cols;
    var squareY = height/nb_rows;
    var squareMin = Math.min(squareX,squareY);
    
    ctx.clearRect(0, 0, width, height);
    ctx.imageSmoothingEnabled = false;
   
    var paddingX = (width - squareMin*nb_cols)/2
    var paddingY = (height - squareMin*nb_rows)/2
    for (var row = 0; row < nb_rows; row++) {
        for (var col = 0; col < nb_cols; col++) {
            ctx.save();
            var black = Module._is_black(g, row, col);
            var ligthed = Module._is_lighted(g, row, col);
            var lightbulb = Module._is_lightbulb(g, row, col);
            var marked = Module._is_marked(g, row, col);
            var error = Module._has_error(g, row, col);

            if(ligthed)
                drawBlankLighted(paddingX+squareMin*col, paddingY+squareMin*row, squareMin, squareMin);
            else
                drawBlank(paddingX+squareMin*col, paddingY+squareMin*row, squareMin, squareMin);

            if (lightbulb)
                drawLightbulb(paddingX+squareMin*col, paddingY+squareMin*row, squareMin, squareMin, error);  
            else if (black)
                drawWall(paddingX+squareMin*col, paddingY+squareMin*row, squareMin, squareMin, Module._get_black_number(g,row,col), error);
            else if (marked)
                drawMark(paddingX+squareMin*col, paddingY+squareMin*row, squareMin, squareMin);
            ctx.restore(); 
        }
    }
}

function drawLightbulb(x, y, width, height, has_error){
    if (has_error) ctx.drawImage(lightbulb_error, x, y, width, height);
    else ctx.drawImage(lightbulb, x, y, width, height);
}

function drawBlank(x, y, width, height){
    ctx.drawImage(blank, x, y, width, height);
}

function drawBlankLighted(x, y, width, height){
    ctx.drawImage(blank_lighted, x, y, width, height);
}

function drawWall(x, y, width, height, wallNumber, has_error){
    ctx.drawImage(wall, x, y, width, height);
    var fontString = 'bold '+Math.min(width-5, height-5)+'px Impact';
    if (wallNumber != -1){
        ctx.font = fontString;
        ctx.fillStyle = 'black';
        ctx.textBaseline = 'middle';
        ctx.textAlign = 'center';
        ctx.fillText(wallNumber, x + (width/2), y + (height/2));
    }
    if (has_error) ctx.drawImage(wall_error, x, y, width, height);
}

function drawMark(x, y, width, height){
    ctx.drawImage(mark, x, y, width, height);
}

function canvasLeftClick(event) {
    event.preventDefault(); // prevent default context menu to appear...
    // get relative cursor position in canvas
    console.log("left click at position:", event.offsetX, event.offsetY);
    var width = canvas.width;
    var height = canvas.height;
    var nb_rows = Module._nb_rows(g);
    var nb_cols = Module._nb_cols(g);
    var squareMin = Math.min((width/nb_cols),(height/nb_rows));
    var paddingX = (width - squareMin*nb_cols)/2
    var paddingY = (height - squareMin*nb_rows)/2
    var col = Math.floor((event.offsetX - paddingX) / ((width-2*paddingX)/nb_cols));
    var row = Math.floor((event.offsetY - paddingY) / ((height-2*paddingY)/nb_rows));
    console.log("game left click coordinates :", row, col);

    if (Module._is_black(g,row,col)) return;
    var square = Module._get_state(g,row,col);
    if (square == S_LIGHTBULB) Module._play_move(g, row, col, S_BLANK);
    else Module._play_move(g, row, col, S_LIGHTBULB);
    drawGame(g);
    win();

}


function canvasRightClick(event) {
    event.preventDefault(); // prevent default context menu to appear...
    // get relative cursor position in canvas
    console.log("right click at position:", event.offsetX, event.offsetY);
    var width = canvas.width;
    var height = canvas.height;
    var nb_rows = Module._nb_rows(g);
    var nb_cols = Module._nb_cols(g);
    var col = Math.floor(event.offsetX / (width/nb_cols));
    var row = Math.floor(event.offsetY / (height/nb_rows));
    console.log("game right click coordinates :", row, col);

    if (Module._is_black(g,row,col)) return;
    var square = Module._get_state(g,row,col);
    if (square == S_MARK) Module._play_move(g, row, col, S_BLANK);
    else Module._play_move(g, row, col, S_MARK);
    drawGame(g);
    win();
}

function restart(){
    Module._restart(g);
    drawGame(g);
    win()
}
function solve(){
    Module._solve(g);
    drawGame(g);
    win();
}
function undo(){
    Module._undo(g);
    drawGame(g);
    win();
}
function redo(){
    Module._redo(g);
    drawGame(g);
    win();
}
function generateRandomFloatInRange(min, max) {
    return Math.floor(Math.random() * (max - min)) + min;
}
function newgame(){
    var choise = document.getElementById("walls%");
    var walls = choise.options[choise.selectedIndex].value;
    var wrap = document.getElementById("wrapping").checked;
    var col= generateRandomFloatInRange(3,9);
    var row= generateRandomFloatInRange(3,9);
    var n_w = walls / 100 * (row * col);
    g= Module._new_random(col,row,wrap,n_w,false);
    win();
    drawGame(g);
}

function resizeCanvas(){
    var size = Math.min(window.innerHeight *0.65, window.innerWidth*0.65) * window.devicePixelRatio;
    canvas.height = size;
    canvas.width = size;
    drawGame(g);
}