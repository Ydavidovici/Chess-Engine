import React from "react";
import { Outlet, NavLink } from "react-router-dom";

export default function Default() {
    return (
        <div className="min-h-screen bg-slate-900 text-slate-100">
            <header className="border-b border-slate-800">
                <div className="mx-auto max-w-6xl px-4 py-3 flex items-center justify-between">
                    <h1 className="text-lg font-semibold tracking-tight">
                        Chess Engine UI
                    </h1>

                    <nav className="flex gap-4 text-sm">
                        <NavLink
                            to="/"
                            className={({ isActive }) =>
                                `hover:text-white ${
                                    isActive ? "text-white" : "text-slate-300"
                                }`
                            }
                        >
                            Home
                        </NavLink>
                        <NavLink
                            to="/game"
                            className={({ isActive }) =>
                                `hover:text-white ${
                                    isActive ? "text-white" : "text-slate-300"
                                }`
                            }
                        >
                            Game
                        </NavLink>
                        <NavLink
                            to="/lichess"
                            className={({ isActive }) =>
                                `hover:text-white ${
                                    isActive ? "text-white" : "text-slate-300"
                                }`
                            }
                        >
                            Lichess
                        </NavLink>
                    </nav>
                </div>
            </header>

            <main className="mx-auto max-w-6xl px-4 py-6">
                <Outlet />
            </main>
        </div>
    );
}