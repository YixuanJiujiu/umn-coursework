// Advertisement images.
firstAd = "../advertisement-images/Nerve.jpg";
secondAd = "../advertisement-images/secret-life-of-pets.jpg";
thirdAd = "../advertisement-images/suicide-squad.jpg";

// Advertisement image tooltips.
firstAdTooltip = "Nerve Fri, Sep 30 7:00PM";
secondAdTooltip = "secret-life-of-pets Fri, Oct 7 7:00PM";
thirdAdTooltip = "suicide-squade Fri, Oct 14 8:00PM";

// Control images.
activeBullet = "../advertisement-images/bullet_blue.png";
nonactiveBullet = "../advertisement-images/bullet_gray.png";
hoverBullet = "../advertisement-images/bullet_orange.png";
next = "../advertisement-images/next_blue.png";
prev = "../advertisement-images/prev_blue.png";
hoverNext = "../advertisement-images/next_orange.png";
hoverPrev = "../advertisement-images/prev_orange.png";

var currentBulletId;    // Holds the element id of the currently selected (active) bullet.
var count;              // Seconds that have elapsed since the page was loaded.
var startTime;          // Used to calculate intervals to show different advertisements.

// Register event handlers for overriding buttons.
function start()
{
    document.getElementById("nextImage").addEventListener("click", goToNext);
    document.getElementById("prevImage").addEventListener("click", goToPrev);
    document.getElementById("bullet1").addEventListener("click", changeToFirstAd);
    document.getElementById("bullet2").addEventListener("click", changeToSecondAd);
    document.getElementById("bullet3").addEventListener("click", changeToThirdAd);
}

function startTimer()
{
    count = 0;
    startTime = 0;
    currentBulletId = "bullet1";    // start with the nerve advertisement
    var doReset = setInterval(cycleImages, 1000);   // update the slideshow every second
}

// This function decides if the current advertisement should be updated or not.
function cycleImages()
{
    var timeElapsed = count - startTime;
    
    if ((timeElapsed == 7) && (currentBulletId == "bullet1"))
    {       // change ad to secret life of pets
            changeToSecondAd();
            count++;
    }
    else if ((timeElapsed == 5) && (currentBulletId == "bullet2"))
    {
            // change ad to suicide squad
            changeToThirdAd();
            count++;
    }
    else if ((timeElapsed == 3) && (currentBulletId == "bullet3"))
    {
            // change ad to nerve
            changeToFirstAd();
            count;
    }
    else
    {
            count++;    // no ad change necessary
    }
}

// Event handler for when the next button is selected. It shows the next
// ad in the slideshow, if the third ad is being shown...it loops back 
// to the first.
function goToNext()
{
    if (currentBulletId == "bullet1")
    {
        changeToSecondAd();
    }
    else if (currentBulletId == "bullet2")
    {
        changeToThirdAd();
    }
    
    else if (currentBulletId == "bullet3")
    {
        changeToFirstAd();   
    }
}

// Event handler for when the previous button is selected. It shows the previous
// ad in the slideshow, if the first is being shown...it loops back to the third.
function goToPrev()
{
    if (currentBulletId == "bullet1")
    {
        changeToThirdAd();
    }
    else if (currentBulletId == "bullet2")
    {
        changeToFirstAd();
    }
    
    else if (currentBulletId == "bullet3")
    {
        changeToSecondAd();   
    }
}

function changeToSecondAd()
{
    document.getElementById("adImage").src = secondAd;                                              // Update advertisement image
    document.getElementById("adImage").title = secondAdTooltip;                                     // Update advertisement tooltip
    document.getElementById("adLink").href = "http://sua.umn.edu/events/calendar/event/14786/";     // Update advedrtisement link
    document.getElementById("bullet1").src = nonactiveBullet;                                       // Update bullet states accordingly
    document.getElementById("bullet2").src = activeBullet;
    document.getElementById("bullet3").src = nonactiveBullet;
    currentBulletId = "bullet2";
    startTime = count;
}
    
function changeToThirdAd()
{
    document.getElementById("adImage").src = thirdAd;
    document.getElementById("adImage").title = thirdAdTooltip;
    document.getElementById("adLink").href = "http://sua.umn.edu/events/calendar/event/14794/";
    document.getElementById("bullet1").src = nonactiveBullet;
    document.getElementById("bullet2").src = nonactiveBullet;
    document.getElementById("bullet3").src = activeBullet;
    currentBulletId = "bullet3";
    startTime = count;
}
    
function changeToFirstAd()
{
    document.getElementById("adImage").src = firstAd;
    document.getElementById("adImage").title = firstAdTooltip;
    document.getElementById("adLink").href = "http://sua.umn.edu/events/calendar/event/14781/";
    document.getElementById("bullet1").src = activeBullet;
    document.getElementById("bullet2").src = nonactiveBullet;
    document.getElementById("bullet3").src = nonactiveBullet;
    currentBulletId = "bullet1";
    startTime = count;
}

// Event handler for when control images are being hovered over.
// When they are hovered over, the images are replaced with their
// orange equivalents (which indicate the hover).
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

// Event handler for when leaving hover of control iamges. Upon
// leaving a hover, the given image will be returned to it's 
// original (non-hover) source.
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

window.addEventListener("load", start, false);              // Start button click event listeners on window load.
document.addEventListener("mouseover", mouseOver, false);   
document.addEventListener("mouseout", mouseOut, false);
window.onload = startTimer;                                 // Start slideshow functionality on window load.