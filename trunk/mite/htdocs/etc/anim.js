
var mover = null;

function doGrid() {

}

function doMove() {
    mover.style.top = parseInt(mover.style.top) + 1 + 'px';
    mover.style.left = parseInt(mover.style.left) + 1 + 'px';
    if (parseInt(mover.style.top) < 200) setTimeout(doMove, 8);
    return;
}

function init() {
    mover = document.getElementById('mover');
    mover.style.left = '-20px';
    mover.style.top = '-20px';
    doMove();
    return;
}

window.onload = init;
