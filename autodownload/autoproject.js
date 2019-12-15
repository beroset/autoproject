(function() {
  /**
   * Check and set a global guard variable.
   * If this content script is injected into the same page again,
   * it will do nothing next time.
   */
  if (window.hasRun) {
    return;
  }
  window.hasRun = true;

  /**
   * Given a URL to a Code Review question, extract the
   * question number and insert an extra pink button
   * next to the C++ tag at the bottom of the question.
   */
  function insertAutoproject() {
    var question=document.querySelector('div#question');
    if (question) {
        var qn=question.getAttribute('data-questionid');
    }
    var myButton = document.createElement('a');
    myButton.setAttribute('class', 'post-tag');
    myButton.setAttribute('qnum', qn);
    myButton.onclick = (function() {
        console.log("sending message to backend");
        console.log(`${qn}`);
        browser.runtime.sendMessage({"qnumber":`${qn}`});
    });
    myButton.style.backgroundColor = "pink";
    myButton.textContent = "AutoProject";
    el=document.querySelector('a.post-tag[href$="c%2b%2b"][rel="tag"]');
    el.insertAdjacentElement('afterend', myButton);
  }
  insertAutoproject();

})();

