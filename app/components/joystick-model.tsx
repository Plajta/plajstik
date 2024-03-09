"use client";

import * as THREE from "three";
import { ThreeElements, useFrame } from "@react-three/fiber";
import { useRef, useState } from "react";

export function JoystickModel(props: ThreeElements["mesh"]) {
    const meshRef = useRef<THREE.Mesh>(null!);

    return (
        <mesh {...props} ref={meshRef} scale={2}>
            <boxGeometry args={[1, 1, 1]} />
            <meshStandardMaterial color="orange" />
        </mesh>
    );
}
