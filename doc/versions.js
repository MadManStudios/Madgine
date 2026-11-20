const versions = [
"0.6.0",
"dev",
];

versions.forEach(function (version) {

    var holder = document.createElement("div");
    var url = window.location.href.replace(/doc\/[^/]+\//, "doc/" + version + "/");
    var dd = document.createElement("dd");
    var link = document.createElement("a");

    link.innerText = version; // or whatever the link should be
    link.href = url;
    dd.appendChild(link)
    holder.appendChild(dd);
    document.write(holder.innerHTML);

});