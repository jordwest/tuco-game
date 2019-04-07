#version 100
precision mediump float;
attribute vec4 aVertexPosition;
attribute vec4 aVertexColor;

uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;

uniform mat4 uTransform;
varying vec4 vColor;

void main() {
    gl_Position = uProjectionMatrix * uModelViewMatrix * (uTransform * aVertexPosition);
    vColor = aVertexColor;
}
