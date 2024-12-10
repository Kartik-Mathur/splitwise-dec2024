// We call useState and useEffect as hooks
// useState: se data change hota hai website par
// useEffect: se hum cpp ke localhost:8080/data se splitwise ka output
// lekar aaenge
import React, { useState, useEffect } from "react";
import "./Splitwise.css";

function Splitwise() {
  const [transactions, setTransactions] = useState([]); // Syntax is  this
  const [error, setError] = useState(""); // Syntax

  useEffect(() => {
    fetch("http://localhost:8080/data")
      .then((response) => {
        if (response.ok) {
          return response.text();
        }
        throw new Error("Network response was not ok.");
      })
      .then((text) => {
        return JSON.parse(text);
      })
      .then((data) => {
        console.log("Data from backend is ", data);
        const transactionList = data.transactions.split("\\n");
        setTransactions(transactionList[0].split("\n"));
      })
      .catch((error) => setError(error.toString()));
  }, []);

  return (
    <div className="splitwise-container">
      <h1>Transactions from Server:</h1>

      {error ? (
        <p className="error-message">Error: {error}</p>
      ) : (
        <ul className="transactions-list">{transactions}</ul>
      )}
    </div>
  );
}

export default Splitwise;
