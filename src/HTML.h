const String postForms = R"===(
<html>
    <head>
        <title>Set Streamer</title>
        <style>
        body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }
        </style>
    </head>
    <body>
        <h1>POST plain text to /getName/</h1><br>
        <form method="GET" action="/getName">
        <input type="text" name="STREAMER">
        <input type="submit" value="Submit\">
        </form>
    </body>
</html>
)===";