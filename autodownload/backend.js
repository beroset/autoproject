
browser.runtime.onMessage.addListener(log_md);

/**
 * Given a question number, return the request URL for 
 * StackExchange API version 2.2.
 */
function make_URL(qnumber) {
    return `https://api.stackexchange.com/2.2/questions/${qnumber}/?order=desc&sort=activity&site=codereview&filter=!)5IYc5cM9scVj-ftqnOnMD(3TmXe`;
}

/**
 * Given a question number, return the markdown for that
 * question from CodeReview.
 */
async function fetch_body(qnumber) {
    const response = await fetch(make_URL(qnumber));
    const myJson = await response.json();
    //return myJson['items'][0]['body_markdown'];
    return myJson['items'][0];
}

function onResponse(response) {
  console.log("Received " + response);
}

function onError(error) {
  console.log(`Error: ${error}`);
}

/**
 * Given a question number message, fetch the associated
 * json and send it to the companion native application.
 * 
 */
function log_md(message) {
    fetch_body(message.qnumber).then(md => {
        var sending = browser.runtime.sendNativeMessage(
            "com.beroset.autoproject",
            md);
        sending.then(onResponse, onError);
    });
}
