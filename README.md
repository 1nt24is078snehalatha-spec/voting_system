# voting_system
PROJECT OVERVIEW
The TechClub Voting System is a web-based polling platform developed using C++ with the Crow
framework for backend services and HTML, CSS, JavaScript for the frontend interface. The system
enables administrators to create voting questions and allows students to participate securely using
email and USN validation. The application ensures fairness by preventing duplicate voting and
storing all data persistently in a JSON file

OBJECTIVES
• To develop a lightweight and efficient voting system using C++.
• To implement secure validation using college email and USN matching.
• To prevent duplicate voting through identity checks.
• To provide real-time vote results with percentage visualization.
• To maintain persistent storage using JSON file handling.

SYSTEM ARCHITECTURE
The system follows a client-server architecture. The frontend (HTML/CSS/JS) communicates with
the backend server via REST API calls. The backend handles business logic, validation, vote
counting, and file storage.

BACKEND COMPONENTS:
• Crow Framework for routing and HTTP handling.
• Boost.Asio for networking.
• VoteInfo and Question structures for data modeling.
• Duplicate vote validation logic.
• JSON-based file storage (data.json).

FRONTEND COMPONENTS:
• Responsive UI using HTML and CSS.
• Dynamic vote rendering using JavaScript.
• Live result bar visualization.
• Admin panel for publishing polls.
• REST API communication using Fetch API.

API ENDPOINTS:
• POST /admin/question – Create a new poll with options.
• GET /latest – Retrieve latest poll and vote results.
• POST /vote – Submit a vote after validation.

SECURITY AND VALIDATION:
The system validates users by matching the first 10 characters of the email and USN. Duplicate
entries are prevented by checking stored vote records. CORS headers are configured to allow
frontend-backend communication securely.

HOW TO RUN PROJECT:
• Install Boost and Crow dependencies.
• Compile the backend C++ file.
• Run the backend server (default port: 18080).
• Open the frontend HTML file in a browser.
• Ensure API URL is set to http://localhost:18080.

FUTURE ENHANCEMENTS
• Database integration (MySQL / PostgreSQL).
• Authentication with admin token security.
• Multiple concurrent polls support.
• Improved UI with dashboards and analytics.
• Deployment on cloud server.
