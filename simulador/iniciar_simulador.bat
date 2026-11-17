@echo off
rem Sirve la carpeta del proyecto en http://localhost:8123 y abre el simulador.
rem Necesario para que el navegador pueda cargar los samples WAV.
cd /d "%~dp0.."
start "" "http://localhost:8123/simulador/beatmaker_sim.html"
python -m http.server 8123
