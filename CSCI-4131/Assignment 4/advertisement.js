var firstAd = "advertisement-images/Nerve.jpg";
var secondAd = "advertisement-images/secret-life-of-pets.jpg";
var thirdAd = "advertisement-images/suicide-squad.jpg";
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
            count++;
            break;
        case 12:
            // change ad to suicide squad
            document.getElementById("adImage").src = thirdAd;
            count++;
            break;
        case 15:
            // change ad to nerve
            document.getElementById("adImage").src = firstAd;
            count = 0;
            break;
        default:
            count++;
            break;
    }
    
    return;
}

//window.addEventListener("load", start, false);
window.onload = startTimer;