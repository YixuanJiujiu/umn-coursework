firstAd = new Image();
secondAd = new Image();
thirdAd = new Image();
activeBullet = new Image();
nonactiveBullet = new Image();
hoverBullet = new Image();
next = new Image();
prev = new Image();
hoverNext = new Image();
hoverPrev = new Image();

firstAd = "advertisement-images/Nerve.jpg";
secondAd = "advertisement-images/secret-life-of-pets.jpg";
thirdAd = "advertisement-images/suicide-squad.jpg";
activeBullet = "advertisement-images/bullet_blue.png";
nonactiveBullet = "advertisement-images/bullet_gray.png";
hoverBullet = "advertisement-images/bullet_orange.png";
next = "advertisement-images/next_blue.png";
prev = "advertisement-images/prev_blue.png";
hoverNext = "advertisement-images/next_orange.png";
hoverPrev = "advertisement-images/prev_orange.png";
var currentBulletId;
var count;

//var advertisements = [];
//advertisements[0] = firstAd;
//advertisements[1] = secondAd;
//advertisements[2] = thirdAd;

// Register event handlers for overriding buttons.
function start()
{
    // event listener for next
    // event listener for prev
    // event listener for bullet1
    // event listener for bullet2
    // event listener for bullet3
}

function startTimer()
{
    count = 0;
    currentBulletId = "bullet1";
    setInterval(cycleImages, 1000);
    return;
}

function cycleImages()
{
    switch (count)
    {
        case 7:
            // change ad to secret life of pets
            document.getElementById("adImage").src = secondAd;
            document.getElementById("bullet1").src = nonactiveBullet;
            document.getElementById("bullet2").src = activeBullet;
            currentBulletId = "bullet2";
            count++;
            break;
        case 12:
            // change ad to suicide squad
            document.getElementById("adImage").src = thirdAd;
            document.getElementById("bullet2").src = nonactiveBullet;
            document.getElementById("bullet3").src = activeBullet;
            currentBulletId = "bullet3";
            count++;
            break;
        case 15:
            // change ad to nerve
            document.getElementById("adImage").src = firstAd;
            document.getElementById("bullet3").src = nonactiveBullet;
            document.getElementById("bullet1").src = activeBullet;
            currentBulletId = "bullet1";
            count = 0;
            break;
        default:
            count++;
            break;
    }
    
    return;
}

function mouseOver(event)
{
    var id = event.target.getAttribute("id");
    if ( id == "bullet1" || id == "bullet2" || id == "bullet3" )
    {
        event.target.setAttribute("src", hoverBullet);
    }
    else if (id == "nextImage")
    {
        event.target.setAttribute("src", hoverNext);
    }
    else if (id == "prevImage")
    {
        event.target.setAttribute("src", hoverPrev);
    }
}

function mouseOut(event)
{
    var id = event.target.getAttribute("id");
    if ( id == "bullet1" || id == "bullet2" || id == "bullet3" )
    {
        if (id == currentBulletId)
        {
            event.target.setAttribute("src", activeBullet);
        }
        else
        {
            event.target.setAttribute("src", nonactiveBullet);
        }
    }
    else if (id == "nextImage")
    {
        event.target.setAttribute("src", next);
    }
    else if (id == "prevImage")
    {
        event.target.setAttribute("src", prev);
    }
}

//window.addEventListener("load", start, false);
document.addEventListener("mouseover", mouseOver, false);
document.addEventListener("mouseout", mouseOut, false);
window.onload = startTimer;